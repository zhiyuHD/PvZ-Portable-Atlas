/*
 * Minimal TCP transport for the experimental PvP mode.  It deliberately
 * transports game actions, never pointer values or renderer state.
 */
#ifndef __PVP_NETWORK_H__
#define __PVP_NETWORK_H__

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <deque>
#include <string>
#include <vector>

enum class PvPCommandKind : uint8_t
{
    SPAWN_ZOMBIE = 1,
    PLANT_ZOMBIE_SUNFLOWER = 2,
    PLANT = 3,
    SYNC_ZOMBIE_SUN = 4,
    SYNC_ZOMBIE_STATE = 5,
    SYNC_ZOMBIE_STATE_END = 6,
    COLLECT_ZOMBIE_SUN = 7,
    SYNC_PROJECTILE_STATE = 8,
    SYNC_PROJECTILE_STATE_END = 9,
    SYNC_STATE_DIGEST = 10,
    REQUEST_STATE_RESYNC = 11,
    SYNC_ZOMBIE_EFFECT_STATE = 12,
    SYNC_PLANT_STATE = 13,
    SYNC_PLANT_STATE_END = 14,
    PLAYER_READY = 15,
    START_MATCH = 16,
    SYNC_PLANT_SUN = 17,
    SYNC_MOWER_STATE = 18
};

struct PvPCommand
{
    PvPCommandKind mKind{};
    int32_t mType{};
    int32_t mRow{};
    int32_t mCol{};
    int32_t mValue{};
};

class PvPNetwork
{
public:
    PvPNetwork(bool theHost, std::string theAddress, uint16_t thePort);
    ~PvPNetwork();

    void Update();
    void Send(const PvPCommand& theCommand);
    bool Pop(PvPCommand& theCommand);
    bool ConsumeConnectionEstablished();
    bool IsHost() const { return mHost; }
    bool IsConnected() const { return mPeerSocket >= 0 && !mClientConnecting; }
    const std::string& GetStatus() const { return mStatus; }

private:
    bool mHost;
    std::string mAddress;
    uint16_t mPort;
    int mListenSocket;
    int mPeerSocket;
    bool mClientConnecting;
    std::chrono::steady_clock::time_point mNextConnectAttempt;
    std::string mStatus;
    std::deque<PvPCommand> mIncoming;
    std::vector<unsigned char> mWriteBuffer;
    size_t mWriteOffset;
    unsigned char mReadBuffer[1024];
    size_t mReadLength;
    bool mConnectionEstablished;

    void CloseSocket(int& theSocket);
    void OpenHost();
    void OpenClient();
    void FinishClientConnect();
    void ClosePeer(bool theRetryClient);
    void AcceptClient();
    void Receive();
    void Flush();
};

#endif
