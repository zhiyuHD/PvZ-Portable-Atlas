// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GameAgentWidget.h"
#include "../../LawnApp.h"
#include "../../GameConstants.h"
#include "../Board.h"
#include "../CursorObject.h"
#include "../Coin.h"
#include "../SeedPacket.h"
#include "widget/EditWidget.h"
#include "widget/Dialog.h"
#include "widget/WidgetManager.h"
#include "graphics/SysFont.h"
#include "Common.h"
#include "../../Resources.h"
#include "GameSelector.h"
#include "SeedChooserScreen.h"
#include "GameButton.h"
#include "../Cutscene.h"
#include <chrono>
#include <algorithm>
#ifdef PVZ_HAS_AGENT_HTTP
#include <nlohmann/json.hpp>
#endif
using namespace Sexy;
namespace {
class AgentEdit : public EditWidget {
    GameAgentWidget* mAgent;
    bool mSecret;
public:
    AgentEdit(int id, GameAgentWidget* agent) : EditWidget(id, agent), mAgent(agent), mSecret(id == 2) {}
    void MouseWheel(int delta) override { mAgent->MouseWheel(delta); }
    void Draw(Graphics* g) override {
        if (!mSecret) { EditWidget::Draw(g); return; }
        g->SetColor(Color(30, 39, 56)); g->FillRect(0, 0, mWidth, mHeight);
        g->SetColor(Color(89, 111, 145)); g->DrawRect(0, 0, mWidth - 1, mHeight - 1);
        g->SetFont(mFont); g->SetColor(Color(229, 235, 245));
        g->DrawString(std::string(std::min<size_t>(mString.size(), 32), '*'), 8, 23);
    }
};
bool Hit(int x, int y, int left, int top, int width, int height) {
    return x >= left && y >= top && x < left + width && y < top + height;
}
void Text(Graphics* g, const std::string& text, int x, int y, Color color = Color(228, 235, 246)) {
    g->SetColor(color); g->DrawString(text, x, y);
}
void Button(Graphics* g, int x, int y, int width, const std::string& label, bool active = true, bool danger = false) {
    g->SetColor(!active ? Color(45, 54, 70) : danger ? Color(166, 61, 74) : Color(51, 103, 218));
    g->FillRect(x, y, width, 30); Text(g, label, x + 10, y + 21);
}
}
GameAgentWidget::GameAgentWidget(LawnApp* app) : mApp(app) {
    mFont = GetSystemCJKFont(15);
    if (!mFont) mFont = FONT_PICO129;
    mWantsFocus = true;
    mHasTransparencies = true;
    Resize(420, 40, 360, 420);
    for (int i = 0; i < 4; ++i) {
        mFields[i] = new AgentEdit(i, this);
        mFields[i]->SetFont(mFont);
        mFields[i]->mMaxChars = i == 3 ? 2000 : 512;
        mFields[i]->SetColor(EditWidget::COLOR_BKG, Color(30, 39, 56));
        mFields[i]->SetColor(EditWidget::COLOR_TEXT, Color(228, 235, 246));
        mFields[i]->SetColor(EditWidget::COLOR_OUTLINE, Color(89, 111, 145));
        mFields[i]->SetColor(EditWidget::COLOR_HILITE, Color(51, 103, 218));
        mFields[i]->Resize(12, i == 3 ? 348 : 100 + i * 66, 336, 32);
        mFields[i]->SetVisible(i == 3);
        AddWidget(mFields[i]);
    }
    std::string endpoint, model;
    app->RegistryReadString("AgentEndpoint", &endpoint);
    app->RegistryReadString("AgentModel", &model);
    mFields[0]->SetText(endpoint);
    mFields[1]->SetText(model);
    Log("你好，我可以进入冒险模式、选卡、开始关卡并游玩对局。点击右上角设置连接模型。", "Agent");
    SetVisible(false);
}
GameAgentWidget::~GameAgentWidget() {
    if (mCancelled) mCancelled->store(true);
    for (auto* field : mFields) { RemoveWidget(field); delete field; }
}
void GameAgentWidget::Toggle() {
    if (mVisible) { Close(); return; }
    SetVisible(true);
    mApp->mWidgetManager->BringToFront(this);
    mApp->mWidgetManager->SetFocus(this);
    MarkDirty();
}
void GameAgentWidget::Close() {
    mStartingAdventure = false;
    InvalidateBoard();
    SetVisible(false);
    mDragging = false;
    if (mApp->mBoard) mApp->mWidgetManager->SetFocus(mApp->mBoard);
    else if (mApp->mGameSelector) mApp->mWidgetManager->SetFocus(mApp->mGameSelector);
}
void GameAgentWidget::InvalidateBoard() {
    // Creating the board for an Agent-started adventure is an expected transition.
    bool enteringAdventure = mStartingAdventure && !mApp->mBoard && mVisible && !mSettings;
    if (!enteringAdventure) { mAutoPlay = false; mStartingAdventure = false; }
    mAutoBoard = nullptr;
    CancelPendingActions();
    mContinue = enteringAdventure;
    if (mCancelled) mCancelled->store(true);
    mDiscard = true;
    mThreatPending = false;
    FinishStream("", "Agent · 已取消");
    mTargetBoard = nullptr;
    mTargetPlantID = 0;
    mApp->mWidgetManager->MarkAllDirty();
}
bool GameAgentWidget::CanAct() const {
    return mApp->mBoard && mApp->mGameScene == GameScenes::SCENE_PLAYING &&
        !mApp->mBoard->mPaused && mApp->mDialogMap.empty() && !mApp->IsPvPMode() &&
        !mApp->IsIZombieLevel() && !mApp->IsCopybookLevel() &&
        mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
        mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM;
}
bool GameAgentWidget::CanStartAdventure() const {
    auto* menu = mApp->mGameSelector;
    return menu && mApp->mGameScene == GameScenes::SCENE_MENU && mApp->mDialogMap.empty() &&
        mApp->mPlayerInfo && !menu->mLoading && menu->mSlideCounter == 0 &&
        menu->mSelectorState == SELECTOR_IDLE && !menu->mStartingGame && !menu->mAdventureButton->mDisabled &&
        !(mApp->IsTrialStageLocked() && (menu->mLevel >= 25 || mApp->HasFinishedAdventure()));
}
bool GameAgentWidget::CanRun() const {
    if (!mApp->mDialogMap.empty() &&
        !(mApp->mDialogMap.size() == 1 && mApp->GetDialog(Dialogs::DIALOG_CONTINUE))) return false;
    if (mApp->mGameSelector && mApp->mGameScene == GameScenes::SCENE_MENU)
        return CanStartAdventure() || mApp->mGameSelector->mStartingGame;
    auto* board = mApp->mBoard;
    return board && (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO || mApp->mGameScene == GameScenes::SCENE_PLAYING) &&
        (!board->mPaused || mApp->GetDialog(Dialogs::DIALOG_CONTINUE)) && !mApp->IsPvPMode() &&
        !mApp->IsIZombieLevel() && !mApp->IsCopybookLevel() &&
        mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN &&
        mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM && mApp->mGameMode != GameMode::GAMEMODE_UPSELL;
}
bool GameAgentWidget::CanChooseSeeds() const {
    auto* chooser = mApp->mSeedChooserScreen;
    return CanRun() && mApp->mDialogMap.empty() && chooser && chooser->mBoard == mApp->mBoard &&
        chooser->mMouseVisible && chooser->mChooseState == CHOOSE_NORMAL &&
        mApp->mBoard->mCutScene && mApp->mBoard->mCutScene->mSeedChoosing;
}
std::string GameAgentWidget::Snapshot() const {
#ifdef PVZ_HAS_AGENT_HTTP
    using Json = nlohmann::json;
    auto* board = mApp->mBoard;
    Json menu = {{"scene", static_cast<int>(mApp->mGameScene)}, {"can_start_adventure", CanStartAdventure()},
        {"adventure_starting", mApp->mGameSelector && mApp->mGameSelector->mStartingGame},
        {"continue_dialog", mApp->GetDialog(Dialogs::DIALOG_CONTINUE) != nullptr},
        {"intro_dialogue", board && board->mCutScene && board->mCutScene->IsShowingCrazyDave() && mApp->mCrazyDaveMessageIndex != -1},
        {"dialogue_index", mApp->mCrazyDaveMessageIndex}, {"can_choose_seeds", CanChooseSeeds()}, {"can_run", CanRun()}};
    if (!board) return Json{{"playing", false}, {"can_act", false}, {"menu", menu},
        {"automatic", mAutoPlay}, {"goal", mPlayGoal}}.dump();
    int rows = board->StageHasPool() ? 6 : 5;
    Json state = {{"playing", mApp->mGameScene == GameScenes::SCENE_PLAYING}, {"can_act", CanAct()},
        {"level", board->mLevel}, {"mode", static_cast<int>(mApp->mGameMode)},
        {"sun", board->mSunMoney}, {"wave", board->mCurrentWave}, {"waves", board->mNumWaves},
        {"rows", rows}, {"cols", 9}, {"shovel_available", board->mShowShovel},
        {"night", board->StageIsNight()}, {"pool", board->StageHasPool()}, {"roof", board->StageHasRoof()},
        {"fog", board->StageHasFog()}, {"tick", board->mMainCounter},
        {"plants", Json::array()}, {"zombies", Json::array()}, {"packets", Json::array()}, {"pickups", Json::array()}};
    state["menu"] = menu;
    if (auto* chooser = mApp->mSeedChooserScreen; chooser && chooser->mBoard == board) {
        Json choices = {{"active", CanChooseSeeds()}, {"capacity", board->mSeedBank->mNumPackets},
            {"selected_count", chooser->mSeedsInBank}, {"seeds_in_flight", chooser->mSeedsInFlight},
            {"extra_page", chooser->mExtraPage}, {"plants", Json::array()}, {"selected", Json::array()},
            {"upcoming_zombie_types", Json::array()}};
        for (int id = 0; id < NUM_SEEDS_IN_CHOOSER; ++id) {
            auto type = static_cast<SeedType>(id);
            if (!mApp->HasSeedType(type)) continue;
            const auto& seed = chooser->mChosenSeeds[id];
            bool selected = seed.mSeedState == SEED_IN_BANK || seed.mSeedState == SEED_FLYING_TO_BANK;
            bool allowed = !chooser->SeedNotAllowedToPick(type) && !chooser->SeedNotAllowedDuringTrial(type);
            choices["plants"].push_back({{"seed", id}, {"name", GetPlantDefinition(type).mPlantName},
                {"cost", GetPlantDefinition(type).mSeedCost}, {"allowed", allowed},
                {"visible", seed.mSeedState != SEED_PACKET_HIDDEN}, {"selected", selected},
                {"recommendation_flags", chooser->SeedNotRecommendedToPick(type)},
                {"can_copy", type < SEED_GATLINGPEA && allowed}});
            if (selected) choices["selected"].push_back({{"packet", seed.mSeedIndexInBank + 1}, {"seed", id},
                {"imitater_seed", static_cast<int>(seed.mImitaterType)}, {"locked_by_dave", seed.mCrazyDavePicked}});
        }
        for (int wave = 0; wave < board->mNumWaves; ++wave)
            for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; ++i) {
                auto type = board->mZombiesInWave[wave][i];
                if (type == ZOMBIE_INVALID) break;
                int id = static_cast<int>(type);
                auto& types = choices["upcoming_zombie_types"];
                if (std::find(types.begin(), types.end(), Json(id)) == types.end()) types.push_back(id);
            }
        state["seed_choices"] = choices;
    }
    Plant* plant = nullptr;
    while (board->IteratePlants(plant)) state["plants"].push_back({{"row", plant->mRow + 1},
        {"col", plant->mPlantCol + 1}, {"seed", static_cast<int>(plant->mSeedType)}, {"health", plant->mPlantHealth}, {"overdraw_suns_remaining", plant->mOverdrawnSuns},
        {"overdraw_cooldown_ticks", plant->mState == PlantState::STATE_OVERDRAWN ? plant->mStateCountdown : 0},
        {"asleep", plant->mIsAsleep}});
    Zombie* zombie = nullptr;
    while (board->IterateZombies(zombie)) state["zombies"].push_back({{"row", zombie->mRow + 1},
        {"x", zombie->mPosX}, {"type", static_cast<int>(zombie->mZombieType)}, {"health", zombie->mBodyHealth}, {"armor_health", zombie->mHelmHealth},
        {"shield_health", zombie->mShieldHealth}, {"speed", zombie->mVelX}, {"phase", static_cast<int>(zombie->mZombiePhase)}});
    Coin* coin = nullptr;
    while (board->IterateCoins(coin)) if (!coin->mIsBeingCollected && (coin->IsSun() || coin->IsMoney()))
        state["pickups"].push_back({{"type", static_cast<int>(coin->mType)}, {"x", coin->mPosX}, {"y", coin->mPosY},
            {"sun_value", coin->IsSun() ? coin->GetSunValue() : 0}});
    if (board->mSeedBank) for (int i = 0; i < board->mSeedBank->mNumPackets; ++i) {
        auto& packet = board->mSeedBank->mSeedPackets[i];
        auto type = packet.mPacketType == SeedType::SEED_IMITATER ? packet.mImitaterType : packet.mPacketType;
        Json cells = Json::array();
        bool valid = static_cast<int>(type) >= 0 && type < SeedType::NUM_SEED_TYPES;
        if (valid) for (int row = 0; row < rows; ++row) for (int col = 0; col < 9; ++col)
            if (board->CanPlantAt(col, row, type) == PlantingReason::PLANTING_OK) cells.push_back({row + 1, col + 1});
        state["packets"].push_back({{"packet", i + 1}, {"seed", static_cast<int>(type)},
            {"name", valid ? GetPlantDefinition(type).mPlantName : "special/empty"},
            {"ready", packet.CanPickUp()}, {"cost", packet.mPacketType == SeedType::SEED_NONE ? 0 : board->GetCurrentPlantCost(packet.mPacketType, packet.mImitaterType)},
            {"cooldown", std::max(0, packet.mRefreshTime - packet.mRefreshCounter)}, {"legal_cells", cells}});
    }
    state["automatic"] = mAutoPlay;
    state["goal"] = mPlayGoal;
    state["recent_events"] = Json::array();
    for (const auto& event : mGameAlerts) if (event.sequence > mSentAlertSequence) {
        Json detail = {{"event", event.event}, {"sequence", event.sequence}, {"row", event.row},
            {"type", event.type}, {"tick", event.tick}};
        if (event.event == "zombie_entered") {
            detail["zombie_id"] = event.id; detail["x"] = event.x; detail["wave"] = event.wave;
        } else {
            detail["plant_id"] = event.id; detail["col"] = event.col; detail["health"] = event.health;
        }
        state["recent_events"].push_back(std::move(detail));
    }
    state["shovel_preview"] = mTargetBoard == board && mTargetPlantID ?
        Json{{"row", mTargetRow + 1}, {"col", mTargetCol + 1}, {"plant_id", mTargetPlantID}} : Json(nullptr);
    return state.dump();
#else
    return "{}";
#endif
}
void GameAgentWidget::SetPage(bool settings) {
    mApp->mWidgetManager->SetFocus(this);
    if (settings) StopAutoPlay();
    mSettings = settings;
    for (int i = 0; i < 4; ++i) mFields[i]->SetVisible(i == 3 ? !settings : settings);
    MarkDirty();
}
void GameAgentWidget::Log(const std::string& line, const std::string& role) {
    if (mToolOutput && role == "工具结果") { *mToolOutput += line + "\n"; }
    mLog.push_back({role, line, ++mNextChatID}); if (mLog.size() > 80) mLog.erase(mLog.begin());
    mScroll = 0; MarkDirty();
}
void GameAgentWidget::AppendMessage(const std::string& message) {
#ifdef PVZ_HAS_AGENT_HTTP
    auto history = nlohmann::json::parse(mConversation);
    history.push_back(nlohmann::json::parse(message));
    // Bound context memory by removing whole assistant/tool exchanges, never orphaning results.
    while (history.size() > 96 && history.front().value("role", "") != "tool") {
        size_t end = 1;
        if (history.front().value("role", "") == "assistant")
            while (end < history.size() && history[end].value("role", "") == "tool") ++end;
        // Keep the current exchange until every tool result has arrived.
        if (end == history.size()) break;
        history.erase(history.begin(), history.begin() + end);
    }
    mConversation = history.dump();
#else
    (void)message;
#endif
}
void GameAgentWidget::CancelPendingActions() {
#ifdef PVZ_HAS_AGENT_HTTP
    for (const auto& action : mPendingActions)
        AppendMessage(nlohmann::json{{"role", "tool"}, {"tool_call_id", action.callID}, {"content", "操作已取消，未执行。"}}.dump());
#endif
    mPendingActions.clear(); mContinue = false;
}
void GameAgentWidget::CancelShovel() {
    bool pending = mTargetBoard != nullptr;
    mTargetBoard = nullptr; mTargetPlantID = 0;
    mApp->mWidgetManager->MarkAllDirty();
    Log(pending ? "已取消铲子预览，植物保留。" : "当前没有铲子预览。", "工具结果");
}
void GameAgentWidget::StopAutoPlay() {
    bool wasPlaying = mAutoPlay;
    bool wasRunning = wasPlaying || mContinue || !mPendingActions.empty() || mRequest.valid();
    mAutoPlay = false; mAutoBoard = nullptr; mStartingAdventure = false;
    mThreatPending = false;
    FinishStream("", "Agent · 已取消");
    CancelPendingActions();
    if (wasRunning) {
        if (mCancelled) mCancelled->store(true);
        mDiscard = true;
        Log(wasPlaying ? "自动游玩已停止。" : "当前任务已停止。");
    }
}
void GameAgentWidget::ClearHistory() {
    StopAutoPlay();
    if (mCancelled) mCancelled->store(true);
    mDiscard = true;
    mTargetBoard = nullptr; mTargetPlantID = 0;
    mLog.clear(); mScroll = 0; mMaxScroll = 0;
    mProgress.reset(); mStreamEntryID = 0; mProgressStatus.clear();
    mGameAlerts.clear(); mSentAlertSequence = mAlertSequence;
    ResetNotices();
    mPlayGoal.clear(); mConversation = "[]";
    mApp->mWidgetManager->MarkAllDirty();
}
void GameAgentWidget::ToggleAutoPlay() {
    if (mAutoPlay) { StopAutoPlay(); return; }
    if (mRequest.valid() || !mPendingActions.empty() || mContinue) { Log("请先停止当前任务后开始自动游玩。"); return; }
    if (!CanRun()) { Log("请进入主菜单、选卡界面或可操作的对局后开始自动游玩。"); return; }
    if (mFields[0]->mString.empty() || mFields[1]->mString.empty() || mFields[2]->mString.empty()) {
        Log("请先配置 BYOK。"); SetPage(true); return;
    }
    mPlayGoal = mFields[3]->mString.empty() ? "进入冒险模式或继续当前关卡，按环境和僵尸选择完整卡组并开始游戏，收集阳光与金币，建立阳光经济，尽力赢得当前对局。" : mFields[3]->mString;
    Log(mPlayGoal, "你"); mFields[3]->SetText("");
    mAutoPlay = true; mAutoBoard = mApp->mBoard; mAutoGameID = mAutoBoard ? mAutoBoard->mGameID : 0;
    mNextThink = std::chrono::steady_clock::now();
    Log("自动游玩已开始；会持续调用 BYOK 接口。点击停止可随时接管。");
}
void GameAgentWidget::Send(bool automatic, bool continuation) {
    if (mRequest.valid() || !mPendingActions.empty() || (mContinue && !continuation)) { StopAutoPlay(); if (mCancelled) mCancelled->store(true); mDiscard = true; Log("正在取消请求…"); return; }
    if (mFields[0]->mString.empty() || mFields[1]->mString.empty() || mFields[2]->mString.empty()) {
        Log("请在设置中填写接口地址、模型和 API Key。"); SetPage(true); return;
    }
    if (!automatic && !continuation && mFields[3]->mString.empty()) { Log("请输入指令，例如：分析局势并种一株植物。"); return; }
    if (!automatic && !continuation) StopAutoPlay();
    std::string prompt;
    if (!continuation) prompt = automatic ? "继续自动游玩。目标：" + mPlayGoal : mFields[3]->mString;
    if (!automatic && !continuation) mPlayGoal = prompt;
#ifdef PVZ_HAS_AGENT_HTTP
    if (!prompt.empty()) AppendMessage(nlohmann::json{{"role", "user"}, {"content", prompt}}.dump());
#endif
    mDiscard = false;
    mRequestBoard = mApp->mBoard;
    mRequestGameID = mRequestBoard ? mRequestBoard->mGameID : 0;
    mRequestPackets.clear();
    if (mRequestBoard && mRequestBoard->mSeedBank) for (int i = 0; i < mRequestBoard->mSeedBank->mNumPackets; ++i) {
        auto& packet = mRequestBoard->mSeedBank->mSeedPackets[i];
        mRequestPackets.emplace_back(static_cast<int>(packet.mPacketType), static_cast<int>(packet.mImitaterType));
    }
    mCancelled = std::make_shared<std::atomic_bool>(false);
    mProgress = std::make_shared<AgentProgress>();
    mProgressRevision = 0; mStreamEntryID = 0; mProgressStatus.clear();
    if (!automatic && !continuation) Log(prompt, "你");
    auto state = Snapshot();
    mSentAlertSequence = mAlertSequence; mThreatPending = false;
    mRequest = std::async(std::launch::async, RequestGameAgent, mFields[0]->mString,
        mFields[2]->mString, mFields[1]->mString, std::string{}, state, mCancelled, mConversation, mProgress);
    if (!automatic && !continuation) mFields[3]->SetText("");
    mContinue = false;
}
void GameAgentWidget::Apply(const AgentAction& action) {
    bool cellTool = action.tool == "place_plant" || action.tool == "hover_shovel" || action.tool == "inspect_cell";
    std::string args = cellTool ? "(row=" + std::to_string(action.row) + ", col=" + std::to_string(action.col) : "(";
    if (action.tool == "place_plant") args += ", packet=" + std::to_string(action.packet);
    if (action.tool == "wait") args += "seconds=" + std::to_string(action.seconds);
    if (action.tool == "select_seed" || action.tool == "deselect_seed" || action.tool == "select_imitater")
        args += "seed=" + std::to_string(action.seed);
    Log(action.tool + args + ")", "工具调用");
    if (action.tool == "get_state" || action.tool == "get_seed_bank" || action.tool == "get_threats" ||
        action.tool == "get_menu_state" || action.tool == "get_seed_choices") {
#ifdef PVZ_HAS_AGENT_HTTP
        auto state = nlohmann::json::parse(Snapshot());
        if (action.tool == "get_seed_bank") Log(state.value("packets", nlohmann::json::array()).dump(), "工具结果");
        else if (action.tool == "get_threats") Log(state.value("zombies", nlohmann::json::array()).dump(), "工具结果");
        else if (action.tool == "get_menu_state") Log(state.value("menu", nlohmann::json::object()).dump(), "工具结果");
        else if (action.tool == "get_seed_choices") Log(state.value("seed_choices", nlohmann::json::object()).dump(), "工具结果");
        else Log(state.dump(), "工具结果");
#else
        Log("此构建未启用状态序列化。", "工具结果");
#endif
        return;
    }
    if (action.tool == "start_adventure") {
        if (!CanStartAdventure()) { Log("未开始：主菜单的冒险入口当前不可用。", "工具结果"); return; }
        mApp->mGameSelector->ClickedAdventure();
        mStartingAdventure = mApp->mGameSelector->mStartingGame;
        Log(mStartingAdventure ? "已点击冒险模式，等待游戏正常进入关卡。" : "冒险模式未开始。", "工具结果");
        return;
    }
    auto* board = mApp->mBoard;
    bool sameBoard = board == mRequestBoard && (!board || board->mGameID == mRequestGameID);
    if (action.tool == "wait") {
        if (!CanRun() || !sameBoard) { Log("等待未执行：游戏已切换或当前不可操作。", "工具结果"); return; }
        mNextThink = std::chrono::steady_clock::now() + std::chrono::seconds(action.seconds);
        Log("等待 " + std::to_string(action.seconds) + " 秒后再决策。", "工具结果"); return;
    }
    if (action.tool == "continue_game") {
        auto* dialog = mApp->GetDialog(Dialogs::DIALOG_CONTINUE);
        if (!CanRun() || !sameBoard || !dialog) { Log("未继续：当前没有可操作的存档继续窗口。", "工具结果"); return; }
        dialog->KeyDown(KeyCode::KEYCODE_RETURN);
        Log("已确认继续现有存档。", "工具结果"); return;
    }
    if (action.tool == "advance_intro") {
        if (!CanRun() || !sameBoard || !board || !mApp->mDialogMap.empty() ||
            mApp->mGameScene != GameScenes::SCENE_LEVEL_INTRO || !board->mCutScene ||
            !board->mCutScene->IsShowingCrazyDave() || mApp->mCrazyDaveMessageIndex == -1) {
            Log("未推进：当前没有可操作的关卡介绍对话。", "工具结果"); return;
        }
        board->mCutScene->KeyDown(KeyCode::KEYCODE_RETURN);
        Log("已按游戏正常流程推进介绍对话，请重新查看状态。", "工具结果"); return;
    }
    bool chooserTool = action.tool == "select_seed" || action.tool == "deselect_seed" ||
        action.tool == "select_imitater" || action.tool == "toggle_seed_page" || action.tool == "start_level";
    if (chooserTool) {
        if (!CanChooseSeeds() || !sameBoard) { Log("选卡操作未执行：选卡界面未就绪或关卡已切换。", "工具结果"); return; }
        auto* chooser = mApp->mSeedChooserScreen;
        // A normal click also lands previous card animations before acting.
        for (int i = 0; i < NUM_SEEDS_IN_CHOOSER; ++i) chooser->LandFlyingSeed(chooser->mChosenSeeds[i]);
        if (action.tool == "toggle_seed_page") {
            chooser->ToggleExtraPage();
            Log(chooser->mExtraPage ? "已切换到额外植物页。" : "已切换到普通植物页。", "工具结果"); return;
        }
        if (action.tool == "start_level") {
            if (chooser->mSeedsInBank != board->mSeedBank->mNumPackets || chooser->mStartButton->mDisabled) {
                Log("未开始：请先选满全部卡槽。", "工具结果"); return;
            }
            chooser->OnStartButton(true);
            Log(board->mCutScene->mSeedChoosing ? "关卡尚未开始，请检查当前状态。" :
                "已确认卡组并开始关卡过场；请等待 can_act 为 true 后种植。", "工具结果"); return;
        }
        if (action.seed < 0 || action.seed >= NUM_SEEDS_IN_CHOOSER) {
            Log("选卡未执行：植物 ID 越界。", "工具结果"); return;
        }
        auto type = static_cast<SeedType>(action.seed);
        if (!mApp->HasSeedType(type)) { Log("选卡未执行：植物尚未解锁。", "工具结果"); return; }
        auto& seed = chooser->mChosenSeeds[type];
        if (action.tool == "deselect_seed") {
            if (seed.mSeedState != SEED_IN_BANK) { Log("此植物当前未入选，无需撤卡。", "工具结果"); return; }
            if (seed.mCrazyDavePicked) { Log("撤卡未执行：此卡由戴夫锁定。", "工具结果"); return; }
            chooser->ClickedSeedInBank(seed);
            chooser->UpdateImitaterButton();
            Log("已撤下植物 " + std::to_string(action.seed) + "。", "工具结果"); return;
        }
        if (chooser->SeedNotAllowedToPick(type) || chooser->SeedNotAllowedDuringTrial(type)) {
            Log("选卡未执行：本关不允许携带此植物。", "工具结果"); return;
        }
        if (action.tool == "select_seed" && seed.mSeedState == SEED_IN_BANK) {
            Log("此植物已经入选，保留原卡槽。", "工具结果"); return;
        }
        if (chooser->mSeedsInBank >= board->mSeedBank->mNumPackets) {
            Log("选卡未执行：卡槽已满，请先撤下其他卡片。", "工具结果"); return;
        }
        if (action.tool == "select_imitater") {
            auto& imitater = chooser->mChosenSeeds[SEED_IMITATER];
            if (!mApp->HasSeedType(SEED_IMITATER) || type >= SEED_GATLINGPEA ||
                chooser->mImitaterButton->mDisabled || imitater.mSeedState != SEED_PACKET_HIDDEN) {
                Log("选卡未执行：模仿者不可用、已经入选或复制目标不是基础植物。", "工具结果"); return;
            }
            imitater.mSeedState = SEED_IN_CHOOSER;
            imitater.mImitaterType = type;
            imitater.mX = chooser->mImitaterButton->mX;
            imitater.mY = chooser->mImitaterButton->mY;
            chooser->ClickedSeedInChooser(imitater);
            chooser->UpdateImitaterButton();
            Log("已选择模仿者复制植物 " + std::to_string(action.seed) + "。", "工具结果"); return;
        }
        if (type == SEED_IMITATER) { Log("请使用 select_imitater 指定模仿者复制目标。", "工具结果"); return; }
        if (seed.mSeedState != SEED_IN_CHOOSER) {
            Log("选卡未执行：植物当前不在可选页，可用 toggle_seed_page 切换额外植物页。", "工具结果"); return;
        }
        chooser->ClickedSeedInChooser(seed);
        Log("已选择植物 " + std::to_string(action.seed) + "，卡槽 " + std::to_string(seed.mSeedIndexInBank + 1) + "。", "工具结果"); return;
    }
    if (action.tool == "cancel_shovel") { CancelShovel(); return; }
    if (action.tool == "confirm_shovel") {
        if (!mTargetBoard) Log("铲除未执行：请先调用 hover_shovel 预览目标。", "工具结果");
        else ConfirmShovel();
        return;
    }
    if (!CanAct() || !board || board != mRequestBoard || board->mGameID != mRequestGameID) {
        Log("操作未执行：对局已切换或当前无法操作。", "工具结果"); return;
    }
    if (action.tool == "collect_sun" || action.tool == "collect_coins") {
        Coin* coin = nullptr; int count = 0;
        while (board->IterateCoins(coin)) {
            bool wanted = action.tool == "collect_sun" ? coin->IsSun() : coin->IsMoney();
            if (!wanted || coin->mIsBeingCollected) continue;
            coin->MouseDown(static_cast<int>(coin->mPosX), static_cast<int>(coin->mPosY), 1);
            if (coin->mIsBeingCollected || coin->mDead) ++count;
        }
        Log("已点击收集 " + std::to_string(count) + (action.tool == "collect_sun" ? " 个阳光。" : " 个金币/钻石。"), "工具结果"); return;
    }
    int row = action.row - 1, col = action.col - 1;
    if (row < 0 || row >= (board->StageHasPool() ? 6 : 5) || col < 0 || col >= 9) {
        Log("操作未执行：坐标越界。", "工具结果"); return;
    }
    if (action.tool == "inspect_cell") {
        Plant* plant = nullptr; int count = 0;
        while (board->IteratePlants(plant)) if (plant->mRow == row && plant->mPlantCol == col) {
            Log("植物 " + std::to_string(static_cast<int>(plant->mSeedType)) + "，生命 " + std::to_string(plant->mPlantHealth), "工具结果"); ++count;
        }
        if (!count) Log("此格没有植物。", "工具结果");
        return;
    }
    if (action.tool == "hover_shovel") {
        if (!board->mShowShovel) { Log("当前对局不可使用铲子。", "工具结果"); return; }
        auto* plant = board->GetTopPlantAt(col, row, PlantPriority::TOPPLANT_DIGGING_ORDER);
        if (!plant) { Log("目标格没有可铲除的植物。", "工具结果"); return; }
        mTargetBoard = board; mTargetGameID = board->mGameID;
        mTargetPlantID = board->mPlants.DataArrayGetID(plant);
        mTargetRow = row; mTargetCol = col;
        Plant* hovered = nullptr;
        while (board->IteratePlants(hovered))
            if (hovered->mRow == row && hovered->mPlantCol == col) hovered->OnShovelHover();
        Log("铲子已悬停：第 " + std::to_string(row + 1) + " 行第 " + std::to_string(col + 1) + " 列；已触发适用的悬停增益，可由 Agent 确认或取消。", "工具结果");
        return;
    }
    int index = action.packet - 1;
    if (!board->mSeedBank || index < 0 || index >= board->mSeedBank->mNumPackets) { Log("无效的植物卡槽。", "工具结果"); return; }
    if (board->mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL) { Log("请先放下当前鼠标工具或卡牌。", "工具结果"); return; }
    auto& packet = board->mSeedBank->mSeedPackets[index];
    auto type = packet.mPacketType == SeedType::SEED_IMITATER ? packet.mImitaterType : packet.mPacketType;
    if (index >= static_cast<int>(mRequestPackets.size()) ||
        mRequestPackets[index] != std::pair{static_cast<int>(packet.mPacketType), static_cast<int>(packet.mImitaterType)} ||
        static_cast<int>(type) < 0 || type >= SeedType::NUM_SEED_TYPES ||
        !packet.CanPickUp() || board->CanPlantAt(col, row, type) != PlantingReason::PLANTING_OK) {
        Log("种植未执行：阳光、冷却或格子条件不满足。", "工具结果"); return;
    }
    int before = board->mPlants.mSize;
    int uses = packet.mTimesUsed;
    bool conveyor = board->HasConveyorBeltSeedBank();
    packet.MouseDown(0, 0, 1);
    if (board->mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_BANK) { Log("此关卡不支持普通卡牌种植。", "工具结果"); return; }
    board->MouseDownWithPlant(board->GridToPixelX(col, row) + 40, board->GridToPixelY(col, row) + 50, 1);
    if (board->mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL) board->RefreshSeedPacketFromCursor();
    Log((conveyor ? board->mPlants.mSize > before : packet.mTimesUsed > uses) ? "已种植：第 " + std::to_string(row + 1) + " 行第 " + std::to_string(col + 1) + " 列。" : "游戏规则拒绝了本次种植。", "工具结果");
}
void GameAgentWidget::ConfirmShovel() {
    auto* board = mApp->mBoard;
    if (!mTargetBoard) return;
    auto id = mTargetPlantID;
    auto* targetBoard = mTargetBoard;
    mTargetBoard = nullptr; mTargetPlantID = 0;
    if (!CanAct() || board != targetBoard || board->mGameID != mTargetGameID || !board->mShowShovel) {
        Log("铲除取消：对局状态已改变。", "工具结果"); return;
    }
    auto* plant = board->mPlants.DataArrayTryToGet(id);
    if (!plant || plant->mDead || plant->mPlantCol != mTargetCol || plant->mRow != mTargetRow ||
        board->GetTopPlantAt(mTargetCol, mTargetRow, PlantPriority::TOPPLANT_DIGGING_ORDER) != plant ||
        board->mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL) {
        Log("铲除取消：目标植物或鼠标工具已改变。", "工具结果"); return;
    }
    int x = board->GridToPixelX(mTargetCol, mTargetRow) + 40;
    int y = board->GridToPixelY(mTargetCol, mTargetRow) + 50;
    // ToolHitTest is authoritative, including flying plants and layered cells.
    if (board->ToolHitTest(x, y) != plant) { Log("铲除取消：实际工具命中目标不同。", "工具结果"); return; }
    board->MouseDownWithTool(x, y, 1, CursorType::CURSOR_TYPE_SHOVEL);
    Log(plant->mDead ? "已确认铲除目标植物。" : "游戏规则拒绝了铲除操作。", "工具结果");
}
void GameAgentWidget::PollProgress() {
    if (!mProgress || mDiscard) return;
    auto now = std::chrono::steady_clock::now();
    if (now < mNextProgressPoll) return;
    mNextProgressPoll = now + std::chrono::milliseconds(50);
    std::string text;
    {
        std::lock_guard lock(mProgress->mutex);
        if (mProgressRevision == mProgress->revision) return;
        mProgressRevision = mProgress->revision;
        text = mProgress->text; mProgressStatus = mProgress->status;
    }
    auto entry = std::find_if(mLog.begin(), mLog.end(), [this](const ChatEntry& item) { return item.id == mStreamEntryID; });
    if (entry != mLog.end()) {
        if (entry->text != text) { entry->text = std::move(text); entry->layoutDirty = true; }
        entry->role = "Agent · 输出中";
    } else if (!text.empty()) {
        Log(text, "Agent · 输出中"); mStreamEntryID = mLog.back().id;
    }
    MarkDirty();
}
void GameAgentWidget::FinishStream(const std::string& text, const std::string& role) {
    auto entry = std::find_if(mLog.begin(), mLog.end(), [this](const ChatEntry& item) { return item.id == mStreamEntryID; });
    if (entry != mLog.end()) {
        entry->role = role;
        if (role == "Agent" && entry->text != text) { entry->text = text; entry->layoutDirty = true; }
        if (entry->text.empty()) mLog.erase(entry);
    } else if (!text.empty()) Log(text, role);
    mStreamEntryID = 0; mProgressStatus.clear();
    MarkDirty();
}
void GameAgentWidget::ResetNotices() {
    std::fill(std::begin(mPendingEntrants), std::end(mPendingEntrants), 0);
    mPendingBites = 0; mPendingEaten = 0; mPendingPlantNotices.clear(); mNoticePending = false;
}
void GameAgentWidget::SyncAlertBoard() {
    auto* board = mApp->mBoard;
    if (board != mAlertBoard || (board && board->mGameID != mAlertGameID)) {
        mAlertBoard = board; mAlertGameID = board ? board->mGameID : 0;
        mGameAlerts.clear(); mAlertedZombies.clear(); mThreatPending = false;
        mPlantBiteTicks.clear(); ResetNotices();
        mNextZombieScan = {};
        mSentAlertSequence = mAlertSequence;
    }
}
void GameAgentWidget::ScheduleGameAlert() {
    // A fixed window avoids both per-tick spam and starvation during a continuous wave.
    if (!mNoticePending) {
        mNoticePending = true;
        mNoticeDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    }
}
void GameAgentWidget::NotifyPlantBite(Plant* plant, bool eaten) {
    if (!plant || plant->mBoard != mApp->mBoard || mApp->mGameScene != GameScenes::SCENE_PLAYING) return;
    SyncAlertBoard();
    auto* board = mApp->mBoard;
    auto id = board->mPlants.DataArrayGetID(plant);
    auto tick = board->mMainCounter;
    if (!eaten) {
        auto [it, first] = mPlantBiteTicks.try_emplace(id, tick);
        auto gap = tick - it->second;
        it->second = tick;
        // Repeated bites by one or many zombies belong to the same eating episode.
        if (!first && gap < 150) return;
        ++mPendingBites;
    } else {
        mPlantBiteTicks.erase(id); ++mPendingEaten;
    }
    GameAlert event{++mAlertSequence, id, plant->mRow + 1, static_cast<int>(plant->mSeedType),
        board->mCurrentWave, tick, 0.0f};
    event.event = eaten ? "plant_eaten" : "plant_under_attack";
    event.col = plant->mPlantCol + 1; event.health = eaten ? 0 : std::max(0, plant->mPlantHealth);
    mGameAlerts.push_back(std::move(event));
    if (mGameAlerts.size() > 128) mGameAlerts.pop_front();
    if (mPendingPlantNotices.size() < 6)
        mPendingPlantNotices.push_back(std::to_string(plant->mRow + 1) + "行" +
            std::to_string(plant->mPlantCol + 1) + "列 " + GetPlantDefinition(plant->mSeedType).mPlantName +
            (eaten ? " 被吃掉" : " 开始被吃"));
    ScheduleGameAlert();
}
void GameAgentWidget::FlushGameAlerts() {
    if (!mNoticePending || std::chrono::steady_clock::now() < mNoticeDeadline) return;
    std::string notice;
    for (int row = 0; row < 6; ++row) if (mPendingEntrants[row]) {
        if (!notice.empty()) notice += "；";
        notice += "第 " + std::to_string(row + 1) + " 行 " + std::to_string(mPendingEntrants[row]) + " 只僵尸入场";
    }
    if (mPendingBites || mPendingEaten) {
        if (!notice.empty()) notice += "\n";
        notice += "植物：" + std::to_string(mPendingBites) + " 株开始被吃，" + std::to_string(mPendingEaten) + " 株被吃掉";
        for (const auto& item : mPendingPlantNotices) notice += "\n" + item;
        if (mPendingBites + mPendingEaten > static_cast<int>(mPendingPlantNotices.size())) notice += "\n其余位置见最新对局状态。";
    }
    if (mVisible && !notice.empty()) Log(notice, "游戏提醒");
    if (mAlertSequence > mSentAlertSequence &&
        (mAutoPlay || mContinue || (mRequest.valid() && !mDiscard) || !mPendingActions.empty())) {
        mThreatPending = true; mNextThink = std::chrono::steady_clock::now();
    }
    ResetNotices();
}
void GameAgentWidget::ObserveZombieEntries() {
    SyncAlertBoard();
    auto* board = mApp->mBoard;
    if (!board || mApp->mGameScene != GameScenes::SCENE_PLAYING) return;
    auto now = std::chrono::steady_clock::now();
    if (now < mNextZombieScan) return;
    mNextZombieScan = now + std::chrono::milliseconds(100);
    Zombie* zombie = nullptr;
    while (board->IterateZombies(zombie)) {
        auto id = static_cast<uint32_t>(board->ZombieGetID(zombie));
        if (!zombie->IsOnBoard() || zombie->IsDeadOrDying() || zombie->mMindControlled ||
            zombie->mFromWave == Zombie::ZOMBIE_WAVE_WINNER || zombie->mRow < 0 ||
            zombie->mRow >= (board->StageHasPool() ? 6 : 5) || zombie->mPosX < 0 || zombie->mPosX >= BOARD_WIDTH)
            continue;
        if (!mAlertedZombies.insert(id).second) continue;
        ++mPendingEntrants[zombie->mRow];
        mGameAlerts.push_back({++mAlertSequence, id, zombie->mRow + 1,
            static_cast<int>(zombie->mZombieType), zombie->mFromWave, board->mMainCounter, zombie->mPosX});
        if (mGameAlerts.size() > 128) mGameAlerts.pop_front();
        ScheduleGameAlert();
    }
    for (auto it = mAlertedZombies.begin(); it != mAlertedZombies.end();)
        if (!board->mZombies.DataArrayTryToGet(*it) || board->mZombies.DataArrayTryToGet(*it)->mDead)
            it = mAlertedZombies.erase(it); else ++it;
    for (auto it = mPlantBiteTicks.begin(); it != mPlantBiteTicks.end();)
        if (board->mMainCounter - it->second >= 150 || !board->mPlants.DataArrayTryToGet(it->first))
            it = mPlantBiteTicks.erase(it); else ++it;
}
void GameAgentWidget::Update() {
    Widget::Update();
    ObserveZombieEntries();
    FlushGameAlerts();
    PollProgress();
    if (mStartingAdventure && mApp->mBoard) {
        mStartingAdventure = false;
        mAutoBoard = mApp->mBoard; mAutoGameID = mAutoBoard->mGameID;
        mContinue = true;
        mNextThink = std::chrono::steady_clock::now();
        Log("已进入关卡，将继续查看介绍、选卡或存档状态。");
    }
    if (mStartingAdventure && !mApp->mBoard &&
        (!mApp->mGameSelector || !mApp->mGameSelector->mStartingGame)) {
        StopAutoPlay();
        Log("冒险入口已离开，请完成当前界面后重新发送指令。");
    }
    if (mRequest.valid() && mRequest.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        auto reply = mRequest.get();
        if (mDiscard || mCancelled->load()) { /* Discarded replies do not repopulate cleared history. */ }
        else if (!reply.error.empty()) { FinishStream("", "Agent · 未完成"); Log(reply.error); StopAutoPlay(); }
        else {
            if (!reply.assistantMessage.empty()) AppendMessage(reply.assistantMessage);
            FinishStream(reply.text, "Agent");
            mPendingActions.assign(reply.actions.begin(), reply.actions.end());
            mNextThink = std::chrono::steady_clock::now() + (reply.actions.empty() ? std::chrono::seconds(2) : std::chrono::seconds(0));
            if (reply.text.empty() && reply.actions.empty()) Log("模型没有返回建议或操作。");
        }
    }
    // Execute one queued call per game tick. Long tool batches never block rendering/input.
    if (!mPendingActions.empty()) {
        auto action = mPendingActions.front();
        mPendingActions.pop_front();
        std::string result;
        mToolOutput = &result;
        Apply(action);
        mToolOutput = nullptr;
#ifdef PVZ_HAS_AGENT_HTTP
        AppendMessage(nlohmann::json{{"role", "tool"}, {"tool_call_id", action.callID},
            {"content", result.empty() ? "操作完成，无附加输出。" : result}}.dump());
#endif
        if (mPendingActions.empty()) mContinue = true;
    }
    if (mTargetBoard && (!CanAct() || mApp->mBoard != mTargetBoard || mApp->mBoard->mGameID != mTargetGameID ||
        !mApp->mBoard->mPlants.DataArrayTryToGet(mTargetPlantID) ||
        mApp->mBoard->mPlants.DataArrayTryToGet(mTargetPlantID)->mDead)) {
        mTargetBoard = nullptr; mTargetPlantID = 0; Log("铲子目标已失效。");
    }
    if (mAutoPlay) {
        if (!CanRun() || mApp->mBoard != mAutoBoard ||
            (mApp->mBoard && mApp->mBoard->mGameID != mAutoGameID) || !mVisible) StopAutoPlay();
    }
    if (mThreatPending && CanAct() && !mRequest.valid() && mPendingActions.empty()) {
        mContinue = true; mNextThink = std::chrono::steady_clock::now();
    }
    if (mVisible && !mSettings && !mStartingAdventure && !mRequest.valid() && mPendingActions.empty() &&
        std::chrono::steady_clock::now() >= mNextThink && (mContinue || mAutoPlay))
        Send(mAutoPlay, mContinue);
    if (mVisible) {
        if (mApp->mDialogMap.empty() && mApp->mWidgetManager->mWidgets.back() != this) mApp->mWidgetManager->BringToFront(this);
        MarkDirty();
    }
}
void GameAgentWidget::Draw(Graphics* g) {
    g->SetFont(mFont);
    g->SetColor(Color(10, 16, 27, 170)); g->FillRect(4, 4, mWidth, mHeight);
    g->SetColor(Color(20, 27, 41)); g->FillRect(0, 0, mWidth, mHeight);
    g->SetColor(Color(31, 42, 61)); g->FillRect(0, 0, mWidth, 38);
    Text(g, mSettings ? "BYOK 设置" : "Agent · Chat", 12, 25);
    Text(g, mSettings ? "返回" : "设置", 268, 25, Color(153, 185, 245));
    Text(g, "×", 334, 25);
    if (mSettings) {
        Text(g, "连接你自己的模型", 12, 64);
        const char* labels[] = {"接口地址（兼容 Chat Completions）", "模型名称", "API Key"};
        for (int i = 0; i < 3; ++i) Text(g, labels[i], 12, 92 + i * 66, Color(153, 173, 201));
        Text(g, "Key 仅在本次运行中保存，隐藏显示。", 12, 292, Color(153, 173, 201));
        Text(g, "请求会向此接口发送当前对局状态。", 12, 314, Color(153, 173, 201));
        Button(g, 12, 342, 336, "保存并返回聊天");
        Text(g, "Esc 关闭 · 拖动标题栏移动", 12, 404, Color(119, 140, 170));
        if (mTargetBoard) DeferOverlay(1000);
        return;
    }
    auto* board = mApp->mBoard;
    Text(g, board ? "阳光 " + std::to_string(board->mSunMoney) + "   波次 " + std::to_string(board->mCurrentWave) + "/" + std::to_string(board->mNumWaves) : "当前没有对局", 12, 60, Color(153, 173, 201));
    // Cache each message's layout; unchanged history never remeasures glyphs.
    int total = 0;
    for (auto& entry : mLog) {
        if (entry.layoutDirty) {
            entry.lines.clear();
            std::string line;
            int width = 0;
            char32_t previous = 0, ch;
            size_t offset = 0;
            // Large state/tool messages keep their full text in model history.
            // Bound the amount of chat layout work for any one message.
            while (offset < entry.text.size() && entry.lines.size() < 200) {
                auto begin = offset;
                if (!UTF8DecodeNext(entry.text, offset, ch)) break;
                if (ch == '\n') {
                    entry.lines.push_back(std::move(line)); line.clear(); width = 0; previous = 0; continue;
                }
                int advance = mFont->CharWidthKern(ch, previous);
                if (!line.empty() && width + advance > 302) {
                    entry.lines.push_back(std::move(line)); line.clear(); width = 0; previous = 0;
                    advance = mFont->CharWidthKern(ch, 0);
                }
                line.append(entry.text, begin, offset - begin); width += advance; previous = ch;
            }
            if (!line.empty()) entry.lines.push_back(std::move(line));
            if (offset < entry.text.size()) entry.lines.push_back("…长消息已折叠，模型仍收到完整内容。");
            entry.height = 30 + static_cast<int>(entry.lines.size()) * 20;
            entry.layoutDirty = false;
        }
        total += entry.height + 8;
    }
    mMaxScroll = std::max(0, total - 198);
    mScroll = std::clamp(mScroll, 0, mMaxScroll);
    Graphics chat(*g); chat.ClipRect(12, 76, 336, 198);
    int y = 76 - std::max(0, total - 198) + mScroll;
    for (const auto& card : mLog) {
        if (y + card.height > 76 && y < 274) {
            chat.SetColor(card.role == "你" ? Color(31, 57, 95) : Color(29, 38, 54));
            chat.FillRect(12, y, 328, card.height);
            Text(&chat, card.role, 22, y + 20, card.role == "工具调用" || card.role == "工具结果" ? Color(239, 185, 110) : Color(153, 185, 245));
            int first = std::max(0, (76 - (y + 42)) / 20);
            for (int i = first; i < static_cast<int>(card.lines.size()); ++i) {
                int baseline = y + 42 + i * 20;
                if (baseline - 20 >= 274) break;
                Text(&chat, card.lines[i], 22, baseline);
            }
        }
        y += card.height + 8;
    }
    if (mMaxScroll > 0) {
        g->SetColor(Color(89, 111, 145));
        g->FillRect(345, 76 + (mMaxScroll - mScroll) * 174 / mMaxScroll, 3, 24);
    }
    Button(g, 12, 278, 158, mAutoPlay ? "停止自动游玩" : "自动游玩", true, mAutoPlay);
    Button(g, 180, 278, 168, "清除历史");
    if (mTargetBoard) {
        Button(g, 12, 310, 222, "确认铲除 " + std::to_string(mTargetRow + 1) + "行 " + std::to_string(mTargetCol + 1) + "列", true, true);
        Button(g, 244, 310, 104, "取消");
    } else {
        Graphics status(*g); status.ClipRect(12, 310, 336, 28);
        Text(&status, !mProgressStatus.empty() ? mProgressStatus :
            (mRequest.valid() || mContinue || !mPendingActions.empty()) ? "Agent 正在执行任务…" :
            mAutoPlay ? "自动游玩中 · 等待下一次决策" : "输入消息 · Enter 发送 · 滚轮查看记录", 12, 330, Color(119, 140, 170));
    }
    Button(g, 12, 386, 100, "查看状态");
    Text(g, CanAct() ? "可操作" : "仅查看", 130, 407, Color(119, 140, 170));
    Button(g, 244, 386, 104, (mRequest.valid() || !mPendingActions.empty() || mContinue) ? "停止任务" : "发送");
    if (mTargetBoard) DeferOverlay(1000);
}
void GameAgentWidget::DrawOverlay(Graphics* g, int) {
    if (!mVisible || !mTargetBoard || mTargetBoard != mApp->mBoard) return;
    g->SetFont(mFont);
    int x = mTargetBoard->GridToPixelX(mTargetCol, mTargetRow) - mX;
    int y = mTargetBoard->GridToPixelY(mTargetCol, mTargetRow) - mY;
    g->SetColor(Color(255, 173, 82, 90)); g->FillRect(x, y, 80, 90);
    g->SetColor(Color(255, 173, 82)); g->DrawRect(x, y, 79, 89);
    Text(g, "铲子 · 待确认", x, y + 20, Color(255, 220, 170));
}
void GameAgentWidget::MouseDown(int x, int y, int button, int) {
    if (button != 0) return;
    if (y < 38) {
        if (x >= 324) { Close(); return; }
        if (x >= 256) { SetPage(!mSettings); return; }
        mDragging = true; mDragX = x; mDragY = y; return;
    }
    if (mSettings) {
        if (Hit(x, y, 12, 342, 336, 30)) {
            mApp->RegistryWriteString("AgentEndpoint", mFields[0]->mString);
            mApp->RegistryWriteString("AgentModel", mFields[1]->mString);
            SetPage(false);
        }
        return;
    }
    if (Hit(x, y, 12, 278, 158, 30)) ToggleAutoPlay();
    else if (Hit(x, y, 180, 278, 168, 30)) ClearHistory();
    else if (Hit(x, y, 244, 386, 104, 30)) Send();
    else if (Hit(x, y, 12, 386, 100, 30)) {
        Log("get_state() · 本地查看", "工具调用");
        auto* board = mApp->mBoard;
        if (!board) Log("当前没有对局。", "工具结果");
        else {
            Log("关卡 " + std::to_string(board->mLevel) + "：植物 " + std::to_string(board->mPlants.mSize) + "，僵尸 " + std::to_string(board->CountZombiesOnScreen()) + "，阳光 " + std::to_string(board->mSunMoney), "工具结果");
            if (board->mSeedBank) for (int i = 0; i < board->mSeedBank->mNumPackets; ++i) {
                auto& packet = board->mSeedBank->mSeedPackets[i];
                Log("卡槽 " + std::to_string(i + 1) + " " + (static_cast<int>(packet.mPacketType) >= 0 && packet.mPacketType < SeedType::NUM_SEED_TYPES ? Plant::GetNameString(packet.mPacketType, packet.mImitaterType) : "特殊卡槽") + (packet.CanPickUp() ? " · 可用" : " · 暂不可用"), "工具结果");
            }
        }
    }
    else if (mTargetBoard && Hit(x, y, 12, 310, 222, 30)) ConfirmShovel();
    else if (mTargetBoard && Hit(x, y, 244, 310, 104, 30)) CancelShovel();
}
void GameAgentWidget::MouseDrag(int x, int y) {
    if (!mDragging) return;
    Move(std::clamp(mX + x - mDragX, 0, std::max(0, mApp->mWidgetManager->mWidth - mWidth)),
        std::clamp(mY + y - mDragY, 0, std::max(0, mApp->mWidgetManager->mHeight - mHeight)));
}
void GameAgentWidget::MouseUp(int, int, int, int) { mDragging = false; }
void GameAgentWidget::MouseWheel(int delta) { if (mSettings) return; mScroll = std::clamp(mScroll + delta * 44, 0, mMaxScroll); MarkDirty(); }
void GameAgentWidget::KeyDown(KeyCode key) { if (key == KEYCODE_ESCAPE) Close(); }
void GameAgentWidget::EditWidgetText(int id, const std::string&) { if (id == 3) Send(); }
