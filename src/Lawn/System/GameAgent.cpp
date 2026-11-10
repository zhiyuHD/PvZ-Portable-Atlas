// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GameAgent.h"
#ifdef PVZ_HAS_AGENT_HTTP
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <map>
#include <thread>
#include <unordered_set>
using Json = nlohmann::json;
namespace {
void Publish(const std::shared_ptr<AgentProgress>& progress, const std::string& text, const std::string& status) {
    if (!progress) return;
    std::lock_guard lock(progress->mutex);
    progress->text = text; progress->status = status; ++progress->revision;
}
struct AgentResponse {
    std::shared_ptr<AgentProgress> progress;
    std::string body, lineBuffer, eventData, text, reasoning;
    std::map<int, Json> calls;
    size_t received = 0;
    long status = 0;
    int retryAfter = -1;
    bool streaming = false, done = false, finished = false, malformed = false, tooLarge = false, streamError = false;

    void Event() {
        auto data = std::move(eventData); eventData.clear();
        if (data.empty()) return;
        if (data == "[DONE]") { done = true; return; }
        auto chunk = Json::parse(data);
        if (chunk.contains("error")) { streamError = true; return; }
        if (!chunk.contains("choices") || chunk["choices"].empty()) return; // usage or keepalive
        for (const auto& choice : chunk.at("choices")) {
            if (choice.value("index", 0) != 0) continue;
            if (choice.contains("finish_reason") && !choice["finish_reason"].is_null()) {
                auto reason = choice["finish_reason"].get<std::string>();
                if (reason == "length" || reason == "content_filter") { streamError = true; return; }
                finished = true;
            }
            if (!choice.contains("delta")) continue;
            const auto& delta = choice["delta"];
            if (delta.contains("content") && delta["content"].is_string()) {
                text += delta["content"].get<std::string>();
                Publish(progress, text, "正在接收回复…");
            }
            if (delta.contains("reasoning_content") && delta["reasoning_content"].is_string())
                reasoning += delta["reasoning_content"].get<std::string>();
            if (delta.contains("tool_calls")) for (const auto& part : delta["tool_calls"]) {
                int index = part.at("index").get<int>();
                if (index < 0) throw std::runtime_error("无效的工具分片");
                auto [it, inserted] = calls.try_emplace(index, Json{{"type", "function"}, {"function", {{"name", ""}, {"arguments", ""}}}});
                auto& call = it->second;
                if (part.contains("id") && part["id"].is_string()) call["id"] = part["id"];
                if (part.contains("function")) for (const char* field : {"name", "arguments"}) {
                    const auto& function = part["function"];
                    if (function.contains(field) && function[field].is_string())
                        call["function"][field] = call["function"][field].get<std::string>() + function[field].get<std::string>();
                }
            }
        }
    }
    void Lines() {
        size_t offset = 0, end;
        while ((end = lineBuffer.find('\n', offset)) != std::string::npos) {
            auto line = lineBuffer.substr(offset, end - offset);
            offset = end + 1;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) Event();
            else if (line.starts_with("data:")) {
                auto value = line.substr(5);
                if (!value.empty() && value.front() == ' ') value.erase(0, 1);
                if (!eventData.empty()) eventData += '\n';
                eventData += value;
            }
        }
        lineBuffer.erase(0, offset);
    }
    void FinishInput() {
        if (!lineBuffer.empty()) { lineBuffer += '\n'; Lines(); }
        Event();
    }
    Json Message() {
        if (!streaming) return Json::parse(body).at("choices").at(0).at("message");
        FinishInput();
        if (streamError) throw std::runtime_error("流式接口报告错误或输出被截断");
        if (!done && !finished) throw std::runtime_error("流式回复提前中断");
        Json message = {{"content", text}, {"tool_calls", Json::array()}};
        if (!reasoning.empty()) message["reasoning_content"] = reasoning;
        for (const auto& [index, call] : calls) message["tool_calls"].push_back(call);
        return message;
    }
};
size_t Receive(char* data, size_t size, size_t count, void* target) {
    auto& response = *static_cast<AgentResponse*>(target);
    auto length = size * count;
    if (length > 1024 * 1024 - response.received) { response.tooLarge = true; return 0; }
    response.received += length;
    try {
        if (!response.streaming) {
            response.body.append(data, length);
            // Also accept SSE from compatible proxies that omit Content-Type.
            auto first = response.body.find_first_not_of(" \t\r\n");
            if (first != std::string::npos && (response.body.compare(first, 5, "data:") == 0 ||
                response.body[first] == ':' || response.body.compare(first, 6, "event:") == 0)) {
                response.streaming = true;
                response.lineBuffer = std::move(response.body);
            } else return length;
        } else response.lineBuffer.append(data, length);
        response.Lines();
        return length;
    } catch (...) { response.malformed = true; return 0; }
}
size_t Header(char* data, size_t size, size_t count, void* target) {
    auto& response = *static_cast<AgentResponse*>(target);
    auto length = size * count;
    try {
        std::string line(data, length), lower = line;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
        if (lower.starts_with("http/")) {
            auto space = lower.find(' ');
            response.status = space == std::string::npos ? 0 : std::stol(lower.substr(space + 1));
            response.streaming = false; response.retryAfter = -1;
        } else if (lower.starts_with("content-type:") && lower.find("text/event-stream") != std::string::npos)
            response.streaming = response.status >= 200 && response.status < 300;
        else if (lower.starts_with("retry-after:")) {
            auto value = line.substr(12);
            auto first = value.find_first_not_of(" \t");
            auto last = value.find_last_not_of(" \t\r\n");
            if (first != std::string::npos) {
                value = value.substr(first, last - first + 1);
                if (std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); }))
                    response.retryAfter = static_cast<int>(std::min<long long>(300, std::stoll(value)));
                else {
                    auto date = curl_getdate(value.c_str(), nullptr);
                    if (date >= 0) response.retryAfter = static_cast<int>(std::clamp<long long>(date - std::time(nullptr), 0, 300));
                }
            }
        }
    } catch (...) { /* Ignore malformed optional HTTP headers. */ }
    return length;
}
bool RetryableStatus(long status) {
    return status == 408 || status == 425 || status == 429 || status == 500 ||
        status == 502 || status == 503 || status == 504;
}
bool RetryableNetwork(CURLcode code) {
    return code == CURLE_COULDNT_RESOLVE_HOST || code == CURLE_COULDNT_RESOLVE_PROXY ||
        code == CURLE_COULDNT_CONNECT || code == CURLE_OPERATION_TIMEDOUT ||
        code == CURLE_RECV_ERROR || code == CURLE_SEND_ERROR || code == CURLE_GOT_NOTHING || code == CURLE_PARTIAL_FILE;
}
bool WaitRetry(const std::shared_ptr<std::atomic_bool>& cancelled, int seconds) {
    auto until = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    while (std::chrono::steady_clock::now() < until) {
        if (cancelled->load()) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return !cancelled->load();
}
int Progress(void* flag, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    return static_cast<std::atomic_bool*>(flag)->load() ? 1 : 0;
}
Json Tool(const char* name, const char* description, Json properties, Json required) {
    return {{"type", "function"}, {"function", {{"name", name}, {"description", description},
        {"parameters", {{"type", "object"}, {"properties", properties}, {"required", required}, {"additionalProperties", false}}}}}};
}
}
#endif
AgentReply RequestGameAgent(const std::string& endpoint, const std::string& key,
    const std::string& model, const std::string& prompt, const std::string& state,
    const std::shared_ptr<std::atomic_bool>& cancelled, const std::string& history,
    const std::shared_ptr<AgentProgress>& progress) {
    AgentReply reply;
#ifdef PVZ_HAS_AGENT_HTTP
    try {
        static std::once_flag init;
        static CURLcode initialized;
        std::call_once(init, [] { initialized = curl_global_init(CURL_GLOBAL_DEFAULT); });
        if (initialized != CURLE_OK) throw std::runtime_error("无法初始化网络");
        auto coordinate = Json{{"type", "integer"}, {"minimum", 1}};
        auto seedID = Json{{"type", "integer"}, {"minimum", 0}};
        auto tools = Json::array({
            Tool("get_state", "Read current match status. State is already included in the request.", Json::object(), Json::array()),
            Tool("get_menu_state", "Inspect current scene, adventure entry, intro dialogue and continue-save dialog.", Json::object(), Json::array()),
            Tool("get_seed_choices", "Inspect unlocked plants, restrictions, recommendations, selected slots and upcoming zombie types before starting.", Json::object(), Json::array()),
            Tool("select_seed", "Select an unlocked plant by its exact zero-based seed ID from get_seed_choices. Does not toggle already selected cards.", {{"seed", seedID}}, {"seed"}),
            Tool("deselect_seed", "Remove a selected card by its zero-based seed ID. Cannot remove Dave's locked choices.", {{"seed", seedID}}, {"seed"}),
            Tool("select_imitater", "Choose the imitater copying an unlocked basic plant. seed is the zero-based ID of the plant to copy.", {{"seed", seedID}}, {"seed"}),
            Tool("toggle_seed_page", "Switch the normal/extra plant page in the seed chooser, exposing unlocked custom plants.", Json::object(), Json::array()),
            Tool("start_level", "Start after filling all seed slots. Accepts the game's deck recommendation warnings on your behalf; check economy, pool/roof support, upgrades and balloon protection first.", Json::object(), Json::array()),
            Tool("start_adventure", "Enter Adventure from the main menu using the normal start button. Wait for the animation and inspect state again.", Json::object(), Json::array()),
            Tool("continue_game", "Accept the existing saved-game Continue dialog and resume that game.", Json::object(), Json::array()),
            Tool("advance_intro", "Advance the current pre-level Dave dialogue using the normal Enter action. Does not skip the seed chooser.", Json::object(), Json::array()),
            Tool("get_seed_bank", "Inspect seed packet names, costs, cooldowns and legal cells.", Json::object(), Json::array()),
            Tool("get_threats", "Inspect zombies by lane including armor and distance to the house.", Json::object(), Json::array()),
            Tool("inspect_cell", "Inspect every plant layer in a cell and its health.", {{"row", coordinate}, {"col", coordinate}}, {"row", "col"}),
            Tool("collect_sun", "Click all available sunlight using normal collection rules.", Json::object(), Json::array()),
            Tool("collect_coins", "Click all silver/gold coins and diamonds. Does not collect level awards.", Json::object(), Json::array()),
            Tool("wait", "Wait before the next automatic decision, for cooldowns or more sun. No blocking of the game.",
                {{"seconds", {{"type", "integer"}, {"minimum", 1}, {"maximum", 15}}}}, {"seconds"}),
            Tool("place_plant", "Place a ready seed packet using normal game rules. Coordinates are 1 based.",
                {{"packet", {{"type", "integer"}, {"minimum", 1}}}, {"row", coordinate}, {"col", coordinate}}, {"packet", "row", "col"}),
            Tool("confirm_shovel", "Confirm and remove the currently previewed plant. You may confirm yourself after checking the preview. Game revalidates target identity.", Json::object(), Json::array()),
            Tool("cancel_shovel", "Cancel the current shovel preview without removing any plant.", Json::object(), Json::array()),
            Tool("hover_shovel", "Hover the shovel over a cell without removing plants. Triggers the game's sunflower overdraw (50 suns then cooldown) and wakes sleeping mushrooms. Use confirm_shovel to remove, cancel_shovel to cancel. You may keep previewing to use this mechanic.",
                {{"row", coordinate}, {"col", coordinate}}, {"row", "col"})});
        Json messages = Json::array({{{"role", "system"}, {"content",
            "You are the built-in PvZ game agent. Reply in Chinese. Only use provided tools. Coordinates and seed packets are 1-based. "
            "Play using normal game rules: collect sunlight, establish an economy, defend each lane, use support plants on pool/roof and account for armor. "
            "You can prepare games yourself: start_adventure from the menu, continue_game for saved games, advance_intro for Dave dialogue, "
            "then get_seed_choices, select_seed/deselect_seed/select_imitater and start_level. Use zero-based seed IDs exactly as reported; row, col and packet remain 1-based. "
            "Choose a full deck appropriate to terrain and upcoming zombies, including economy and upgrade prerequisites. Dave-locked cards cannot be removed. "
            "start_level accepts deck recommendation warnings, so evaluate your deck before using it. During transitions use wait and inspect state; do not plant until can_act is true. "
            "Tool calls run in order on the game thread and actual results return as tool messages. There is no fixed tool-call or decision-round limit. "
            "Prefer short adaptive batches so you can observe changes. After tools, continue observing and acting until the requested task is complete. "
            "In automatic mode keep playing until stopped or the match ends. Use wait for cooldowns/resources. "
            "hover_shovel does not remove a plant: it triggers this game's sunflower overdraw (50 suns then 3600 ticks of cooldown), and wakes sleeping mushrooms. "
            "You may confirm removal yourself with confirm_shovel or cancel with cancel_shovel. Confirm acts on the current preview, revalidating plant identity. "
            "recent_events includes zombie_entered, plant_under_attack (eating starts) and plant_eaten (confirmed loss). "
            "React to these alerts before resuming a wait: defend threatened lanes, protect plants being eaten and replace lost defenses or economy. "
            "Plant events include exact plant_id, zero-based seed type, 1-based row/col and remaining health. "
            "Use current snapshots over stale history. Never claim success until the actual result. Current match state: " + state}}});
        auto previous = Json::parse(history);
        if (!previous.is_array()) throw std::runtime_error("对话历史格式无效");
        for (const auto& item : previous) messages.push_back(item);
        if (!prompt.empty()) messages.push_back({{"role", "user"}, {"content", prompt}});
        Json body = {{"model", model}, {"stream", true}, {"tools", tools}, {"messages", messages}};
        std::string url = endpoint;
        while (!url.empty() && url.back() == '/') url.pop_back();
        if (!url.ends_with("/chat/completions")) url += "/chat/completions";
        if (key.find_first_of("\r\n") != std::string::npos) throw std::runtime_error("API Key 格式无效");
        auto curl = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>(curl_easy_init(), curl_easy_cleanup);
        if (!curl) throw std::runtime_error("无法创建网络请求");
        curl_slist* rawHeaders = nullptr;
        rawHeaders = curl_slist_append(rawHeaders, "Content-Type: application/json");
        rawHeaders = curl_slist_append(rawHeaders, ("Authorization: Bearer " + key).c_str());
        auto headers = std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>(rawHeaders, curl_slist_free_all);
        auto payload = body.dump();
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 180L);
        curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, 60L);
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, Receive);
        curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, Header);
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, Progress);
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, cancelled.get());
        Json message;
        constexpr int maxRetries = 5;
        for (int attempt = 0; ; ++attempt) {
            if (cancelled->load()) throw std::runtime_error("请求已取消");
            Publish(progress, "", attempt ? "正在重新请求…" : "正在连接模型…");
            AgentResponse response; response.progress = progress;
            curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &response);
            auto result = curl_easy_perform(curl.get());
            if (cancelled->load()) throw std::runtime_error("请求已取消");
            long status = 0;
            curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);
            if (result == CURLE_OK && response.streaming) {
                try { response.FinishInput(); }
                catch (...) { response.malformed = true; }
            }
            bool interrupted = result == CURLE_OK && status >= 200 && status < 300 &&
                response.streaming && !response.done && !response.finished && !response.streamError;
            bool retry = !response.malformed && !response.tooLarge &&
                (RetryableStatus(status) || RetryableNetwork(result) || interrupted);
            if (retry && attempt < maxRetries) {
                int delay = response.retryAfter >= 0 ? response.retryAfter : (1 << attempt);
                delay = std::max(1, delay);
                std::string reason = status >= 400 ? "HTTP " + std::to_string(status) : "连接暂时中断";
                Publish(progress, "", reason + "，" + std::to_string(delay) + " 秒后重试（" +
                    std::to_string(attempt + 1) + "/" + std::to_string(maxRetries) + "）…");
                if (!WaitRetry(cancelled, delay)) throw std::runtime_error("请求已取消");
                continue;
            }
            if (response.tooLarge) throw std::runtime_error("接口回复过大");
            if (response.malformed) throw std::runtime_error("流式接口响应格式无效");
            if (result != CURLE_OK) throw std::runtime_error("网络请求失败，自动重试已结束或错误不可重试");
            if (status < 200 || status >= 300) throw std::runtime_error("接口返回 HTTP " + std::to_string(status) +
                (RetryableStatus(status) ? "，自动重试次数已用完" : "，请检查 Key、模型和地址"));
            message = response.Message();
            break;
        }
        if (message.contains("content") && message["content"].is_string()) reply.text = message["content"].get<std::string>();

        std::unordered_set<std::string> callIDs;
        if (message.contains("tool_calls")) for (auto& call : message["tool_calls"]) {
            auto& function = call.at("function");
            auto name = function.at("name").get<std::string>();
            if (name != "get_state" && name != "get_seed_bank" && name != "get_threats" &&
                name != "inspect_cell" && name != "collect_sun" && name != "collect_coins" &&
                name != "wait" && name != "place_plant" && name != "hover_shovel" &&
                name != "confirm_shovel" && name != "cancel_shovel" && name != "get_menu_state" &&
                name != "get_seed_choices" && name != "select_seed" && name != "deselect_seed" &&
                name != "select_imitater" && name != "toggle_seed_page" && name != "start_level" &&
                name != "start_adventure" && name != "continue_game" && name != "advance_intro")
                throw std::runtime_error("模型返回了不支持的工具");
            auto arguments = Json::parse(function.at("arguments").get<std::string>());
            if (!arguments.is_object()) throw std::runtime_error("工具参数必须是对象");
            auto integer = [&](const char* field, int maximum) {
                const auto& value = arguments.at(field);
                if (!value.is_number_integer()) throw std::runtime_error("工具参数必须是整数");
                auto number = value.get<int64_t>();
                if (number < 1 || number > maximum) throw std::runtime_error("工具参数超出范围");
                return static_cast<int>(number);
            };
            AgentAction action; action.tool = name;
            // Normalize missing IDs from compatible endpoints for the tool-result round trip.
            if (!call.contains("id") || !call["id"].is_string() || call["id"].get<std::string>().empty() || callIDs.contains(call["id"].get<std::string>()))
                call["id"] = "game_call_" + std::to_string(reply.actions.size());
            action.callID = call["id"].get<std::string>();
            while (callIDs.contains(action.callID)) action.callID += "_";
            call["id"] = action.callID;
            callIDs.insert(action.callID);
            call["type"] = "function";
            if (name == "place_plant" || name == "hover_shovel" || name == "inspect_cell") {
                action.row = integer("row", 6);
                action.col = integer("col", 9);
                if (name == "place_plant") action.packet = integer("packet", 10);
            }
            if (name == "wait") action.seconds = integer("seconds", 15);
            if (name == "select_seed" || name == "deselect_seed" || name == "select_imitater") {
                const auto& value = arguments.at("seed");
                if (!value.is_number_integer()) throw std::runtime_error("植物 ID 必须是整数");
                auto number = value.get<int64_t>();
                if (number < 0 || number >= 52) throw std::runtime_error("植物 ID 超出选卡范围");
                action.seed = static_cast<int>(number);
            }
            reply.actions.push_back(action);
        }
        Json assistant = {{"role", "assistant"}, {"content", reply.text.empty() ? Json(nullptr) : Json(reply.text)}};
        if (message.contains("reasoning_content") && message["reasoning_content"].is_string())
            assistant["reasoning_content"] = message["reasoning_content"];
        if (!reply.actions.empty()) assistant["tool_calls"] = message["tool_calls"];
        reply.assistantMessage = assistant.dump();
    } catch (const std::exception& error) {
        reply.actions.clear();
        // Parser exceptions may include server text; do not expose it or credentials.
        reply.error = dynamic_cast<const nlohmann::json::exception*>(&error) ? "接口响应格式无效" : error.what();
    }
#else
    (void)endpoint; (void)key; (void)model; (void)prompt; (void)state; (void)cancelled; (void)history; (void)progress;
    reply.error = "此构建未启用 Agent 网络支持 (PVZ_AGENT_HTTP)";
#endif
    return reply;
}
