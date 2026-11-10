// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <atomic>
#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct AgentAction {
    std::string tool, callID;
    int row = 0, col = 0, packet = -1, seconds = 2;
    int seed = -1;
};
struct AgentReply {
    std::string text, error, assistantMessage;
    std::vector<AgentAction> actions;
};
// Worker-owned updates, copied by the GUI under the mutex on the game thread.
struct AgentProgress {
    std::mutex mutex;
    std::string text, status;
    uint64_t revision = 0;
};
// No game objects cross the worker boundary. All actions run on the game thread.
AgentReply RequestGameAgent(const std::string& endpoint, const std::string& key,
    const std::string& model, const std::string& prompt, const std::string& state,
    const std::shared_ptr<std::atomic_bool>& cancelled, const std::string& history = "[]",
    const std::shared_ptr<AgentProgress>& progress = {});
