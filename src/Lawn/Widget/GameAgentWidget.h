// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "widget/Widget.h"
#include "widget/EditListener.h"
#include "../System/GameAgent.h"
#include <chrono>
#include <deque>
#include <unordered_set>
#include <unordered_map>
class LawnApp;
class Board;
class Plant;
namespace Sexy { class EditWidget; }

class GameAgentWidget : public Sexy::Widget, public Sexy::EditListener {
public:
    explicit GameAgentWidget(LawnApp* app);
    ~GameAgentWidget() override;
    void Toggle();
    void InvalidateBoard();
    void NotifyPlantBite(Plant* plant, bool eaten = false);
    void Draw(Sexy::Graphics* g) override;
    void DrawOverlay(Sexy::Graphics* g, int priority) override;
    void Update() override;
    void KeyDown(Sexy::KeyCode key) override;
    void MouseDown(int x, int y, int button, int clicks) override;
    void MouseDrag(int x, int y) override;
    void MouseUp(int x, int y, int button, int clicks) override;
    void MouseWheel(int delta) override;
    void EditWidgetText(int id, const std::string& text) override;
private:
    LawnApp* mApp;
    Sexy::EditWidget* mFields[4];
    Sexy::_Font* mFont;
    struct ChatEntry {
        std::string role, text;
        uint64_t id;
        std::vector<std::string> lines;
        int height = 0;
        bool layoutDirty = true;
    };
    std::vector<ChatEntry> mLog;
    uint64_t mNextChatID = 0, mStreamEntryID = 0, mProgressRevision = 0;
    std::shared_ptr<AgentProgress> mProgress;
    std::string mProgressStatus;
    std::chrono::steady_clock::time_point mNextProgressPoll{};
    struct GameAlert {
        uint64_t sequence; uint32_t id; int row, type, wave; uint32_t tick; float x;
        std::string event = "zombie_entered";
        int col = 0, health = 0;
    };
    std::deque<GameAlert> mGameAlerts;
    std::unordered_set<uint32_t> mAlertedZombies;
    Board* mAlertBoard = nullptr;
    intptr_t mAlertGameID = 0;
    uint64_t mAlertSequence = 0, mSentAlertSequence = 0;
    bool mThreatPending = false;
    int mPendingEntrants[6] = {}, mPendingBites = 0, mPendingEaten = 0;
    std::vector<std::string> mPendingPlantNotices;
    std::unordered_map<uint32_t, uint32_t> mPlantBiteTicks;
    std::chrono::steady_clock::time_point mNoticeDeadline{};
    std::chrono::steady_clock::time_point mNextZombieScan{};
    bool mNoticePending = false;
    bool mSettings = false, mAutoPlay = false, mContinue = false;
    bool mStartingAdventure = false;
    std::string mConversation = "[]";
    std::deque<AgentAction> mPendingActions;
    std::string* mToolOutput = nullptr;
    std::string mPlayGoal;
    Board* mAutoBoard = nullptr;
    intptr_t mAutoGameID = 0;
    std::chrono::steady_clock::time_point mNextThink{};
    int mMaxScroll = 0;
    int mScroll = 0, mDragX = 0, mDragY = 0;
    bool mDragging = false, mDiscard = false;
    std::future<AgentReply> mRequest;
    std::shared_ptr<std::atomic_bool> mCancelled;
    Board* mRequestBoard = nullptr;
    intptr_t mRequestGameID = 0;
    Board* mTargetBoard = nullptr;
    intptr_t mTargetGameID = 0;
    uint32_t mTargetPlantID = 0;
    std::vector<std::pair<int, int>> mRequestPackets;
    int mTargetRow = 0, mTargetCol = 0;
    void Close();
    void Send(bool automatic = false, bool continuation = false);
    void AppendMessage(const std::string& message);
    void CancelPendingActions();
    void CancelShovel();
    void StopAutoPlay();
    void ToggleAutoPlay();
    void ClearHistory();
    void SetPage(bool settings);
    void Log(const std::string& line, const std::string& role = "系统");
    void Apply(const AgentAction& action);
    void ConfirmShovel();
    void ObserveZombieEntries();
    void SyncAlertBoard();
    void FlushGameAlerts();
    void ScheduleGameAlert();
    void ResetNotices();
    void PollProgress();
    void FinishStream(const std::string& text, const std::string& role);
    bool CanAct() const;
    bool CanChooseSeeds() const;
    bool CanRun() const;
    bool CanStartAdventure() const;
    std::string Snapshot() const;
};
