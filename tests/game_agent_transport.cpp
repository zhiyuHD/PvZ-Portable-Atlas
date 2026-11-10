// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Lawn/System/GameAgent.h"
#include <cassert>
#include <iostream>
int main(int argc, char** argv) {
    assert(argc == 2);
    auto cancel = std::make_shared<std::atomic_bool>(false);
    auto call = [&](const char* path) { return RequestGameAgent(std::string(argv[1]) + path, "mock-key", "mock-model", "分析", "{}", cancel); };
    auto reply = call("/valid");
    assert(reply.error.empty() && reply.actions.size() == 1 && reply.actions[0].tool == "hover_shovel");
    assert(reply.actions[0].row == 2 && reply.actions[0].col == 3);
    reply = call("/plant");
    assert(reply.error.empty() && reply.actions[0].packet == 1);
    reply = call("/unknown"); assert(!reply.error.empty() && reply.actions.empty());
    reply = call("/multiple"); assert(reply.error.empty() && reply.actions.size() == 2);
    reply = call("/badjson"); assert(!reply.error.empty() && reply.actions.empty());
    reply = call("/unauthorized"); assert(reply.error.find("401") != std::string::npos);
    cancel->store(true); reply = call("/valid"); assert(!reply.error.empty());
    std::cout << "Transport checks passed\n";
}
