#include "PvPNetwork.h"

#include <cerrno>
#include <cstring>
#include <utility>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace
{
constexpr uint32_t PVP_MAGIC = 0x50565A50U; // PVZP
constexpr uint8_t PVP_VERSION = 13;
// State snapshots can temporarily accumulate when a window loses focus or
// the OS pauses its socket scheduling.  Keep enough room for recovery
// instead of treating a few seconds of normal backlog as a disconnect.
constexpr size_t PVP_MAX_WRITE_BACKLOG = 4 * 1024 * 1024;

#pragma pack(push, 1)
struct Packet
{
    uint32_t magic;
    uint8_t version;
    uint8_t kind;
    int32_t type;
    int32_t row;
    int32_t col;
    int32_t value;
};
#pragma pack(pop)

void SetNonBlocking(int socket)
{
#if defined(_WIN32)
    u_long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
#else
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags >= 0)
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
#endif
}

void CloseNativeSocket(int socket)
{
#if defined(_WIN32)
    closesocket(socket);
#else
    close(socket);
#endif
}

bool IsWouldBlock()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

bool IsConnectInProgress()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS || error == WSAEALREADY;
#else
    return errno == EINPROGRESS || errno == EALREADY || errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

int SendFlags()
{
#if defined(MSG_NOSIGNAL)
    return MSG_NOSIGNAL;
#else
    return 0;
#endif
}

Packet EncodePacket(const PvPCommand& command)
{
    return {
        htonl(PVP_MAGIC),
        PVP_VERSION,
        static_cast<uint8_t>(command.mKind),
        static_cast<int32_t>(htonl(static_cast<uint32_t>(command.mType))),
        static_cast<int32_t>(htonl(static_cast<uint32_t>(command.mRow))),
        static_cast<int32_t>(htonl(static_cast<uint32_t>(command.mCol))),
        static_cast<int32_t>(htonl(static_cast<uint32_t>(command.mValue)))
    };
}
}

PvPNetwork::PvPNetwork(bool theHost, std::string theAddress, uint16_t thePort)
    : mHost(theHost), mAddress(std::move(theAddress)), mPort(thePort), mListenSocket(-1),
      mPeerSocket(-1), mClientConnecting(false), mNextConnectAttempt(), mStatus("Starting"),
      mWriteOffset(0), mReadLength(0), mConnectionEstablished(false)
{
#if defined(_WIN32)
    WSADATA data{};
    WSAStartup(MAKEWORD(2, 2), &data);
#endif
    if (mHost)
        OpenHost();
    else
        OpenClient();
}

PvPNetwork::~PvPNetwork()
{
    CloseSocket(mPeerSocket);
    CloseSocket(mListenSocket);
#if defined(_WIN32)
    WSACleanup();
#endif
}

void PvPNetwork::CloseSocket(int& theSocket)
{
    if (theSocket >= 0)
    {
        CloseNativeSocket(theSocket);
        theSocket = -1;
    }
}

void PvPNetwork::OpenHost()
{
    mListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mListenSocket < 0)
    {
        mStatus = "TCP listen socket failed";
        return;
    }
    int enabled = 1;
    setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enabled), sizeof(enabled));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(mPort);
    if (bind(mListenSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || listen(mListenSocket, 1) < 0)
    {
        mStatus = "TCP listen failed";
        CloseSocket(mListenSocket);
        return;
    }
    SetNonBlocking(mListenSocket);
    mStatus = "Waiting for zombie player";
}

void PvPNetwork::OpenClient()
{
    mPeerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mPeerSocket < 0)
    {
        mStatus = "TCP client socket failed";
        mNextConnectAttempt = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        return;
    }
    SetNonBlocking(mPeerSocket);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(mPort);
    if (inet_pton(AF_INET, mAddress.c_str(), &address.sin_addr) != 1)
    {
        mStatus = "TCP invalid host address";
        ClosePeer(true);
        return;
    }
    if (connect(mPeerSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0)
    {
        mStatus = "Connected to plant player";
        mConnectionEstablished = true;
        return;
    }
    if (IsConnectInProgress())
    {
        mClientConnecting = true;
        mStatus = "Connecting to plant player";
        return;
    }
    mStatus = "TCP connect failed";
    ClosePeer(true);
}

void PvPNetwork::FinishClientConnect()
{
    if (!mClientConnecting || mPeerSocket < 0)
        return;

    fd_set writeSet;
    fd_set errorSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(mPeerSocket, &writeSet);
    FD_SET(mPeerSocket, &errorSet);
    timeval timeout{};
    int ready = select(mPeerSocket + 1, nullptr, &writeSet, &errorSet, &timeout);
    if (ready <= 0)
        return;

    int error = 0;
#if defined(_WIN32)
    int errorLength = sizeof(error);
#else
    socklen_t errorLength = sizeof(error);
#endif
    getsockopt(mPeerSocket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &errorLength);
    if (error == 0 && FD_ISSET(mPeerSocket, &writeSet))
    {
        mClientConnecting = false;
        mStatus = "Connected to plant player";
        mConnectionEstablished = true;
        return;
    }
    mStatus = "TCP connect failed";
    ClosePeer(true);
}

void PvPNetwork::ClosePeer(bool theRetryClient)
{
    CloseSocket(mPeerSocket);
    mClientConnecting = false;
    mWriteBuffer.clear();
    mWriteOffset = 0;
    mReadLength = 0;
    mIncoming.clear();
    mConnectionEstablished = false;
    if (theRetryClient && !mHost)
        mNextConnectAttempt = std::chrono::steady_clock::now() + std::chrono::seconds(2);
}

void PvPNetwork::AcceptClient()
{
    if (mListenSocket < 0 || mPeerSocket >= 0)
        return;
    int accepted = accept(mListenSocket, nullptr, nullptr);
    if (accepted >= 0)
    {
        mPeerSocket = accepted;
        SetNonBlocking(mPeerSocket);
        mStatus = "Zombie player connected";
        mConnectionEstablished = true;
    }
}

void PvPNetwork::Update()
{
    if (mHost)
        AcceptClient();
    else if (mPeerSocket < 0)
    {
        if (std::chrono::steady_clock::now() >= mNextConnectAttempt)
            OpenClient();
    }
    else
    {
        FinishClientConnect();
    }
    if (!IsConnected())
        return;
    Receive();
    Flush();
}

void PvPNetwork::Send(const PvPCommand& theCommand)
{
    if (mPeerSocket >= 0)
    {
        if (mWriteBuffer.size() - mWriteOffset + sizeof(Packet) > PVP_MAX_WRITE_BACKLOG)
        {
            mStatus = "TCP send backlog overflow";
            ClosePeer(true);
            return;
        }
        Packet packet = EncodePacket(theCommand);
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&packet);
        mWriteBuffer.insert(mWriteBuffer.end(), bytes, bytes + sizeof(packet));
    }
}

bool PvPNetwork::Pop(PvPCommand& theCommand)
{
    if (mIncoming.empty())
        return false;
    theCommand = mIncoming.front();
    mIncoming.pop_front();
    return true;
}

bool PvPNetwork::ConsumeConnectionEstablished()
{
    bool established = mConnectionEstablished;
    mConnectionEstablished = false;
    return established;
}

void PvPNetwork::Receive()
{
    if (mPeerSocket < 0)
        return;

    auto processPackets = [this]()
    {
        while (mReadLength >= sizeof(Packet))
        {
            Packet packet{};
            memcpy(&packet, mReadBuffer, sizeof(packet));
            memmove(mReadBuffer, mReadBuffer + sizeof(packet), mReadLength - sizeof(packet));
            mReadLength -= sizeof(packet);
            if (ntohl(packet.magic) != PVP_MAGIC || packet.version != PVP_VERSION ||
                packet.kind < static_cast<uint8_t>(PvPCommandKind::SPAWN_ZOMBIE) ||
                packet.kind > static_cast<uint8_t>(PvPCommandKind::SYNC_MOWER_STATE))
            {
                mStatus = "TCP protocol mismatch (version=" + std::to_string(packet.version) +
                    ", command=" + std::to_string(packet.kind) + ")";
                ClosePeer(true);
                return false;
            }
            mIncoming.push_back({
                static_cast<PvPCommandKind>(packet.kind),
                static_cast<int32_t>(ntohl(static_cast<uint32_t>(packet.type))),
                static_cast<int32_t>(ntohl(static_cast<uint32_t>(packet.row))),
                static_cast<int32_t>(ntohl(static_cast<uint32_t>(packet.col))),
                static_cast<int32_t>(ntohl(static_cast<uint32_t>(packet.value)))
            });
        }
        return true;
    };

    for (;;)
    {
        int remaining = static_cast<int>(sizeof(mReadBuffer) - mReadLength);
        if (remaining == 0)
        {
            mStatus = "TCP packet overflow";
			ClosePeer(true);
            return;
        }
        int received = recv(mPeerSocket, reinterpret_cast<char*>(mReadBuffer + mReadLength), remaining, 0);
        if (received == 0)
        {
            mStatus = "TCP peer disconnected";
            ClosePeer(true);
            return;
        }
        if (received < 0)
        {
            if (IsWouldBlock())
                break;
            mStatus = "TCP receive failed";
            ClosePeer(true);
            return;
        }
        mReadLength += static_cast<size_t>(received);
        if (!processPackets())
            return;
    }
}

void PvPNetwork::Flush()
{
    while (mPeerSocket >= 0 && mWriteOffset < mWriteBuffer.size())
    {
        int sent = send(mPeerSocket, reinterpret_cast<const char*>(mWriteBuffer.data() + mWriteOffset),
            static_cast<int>(mWriteBuffer.size() - mWriteOffset), SendFlags());
        if (sent > 0)
        {
            mWriteOffset += static_cast<size_t>(sent);
            continue;
        }
        if (sent < 0 && IsWouldBlock())
            return;
        mStatus = "TCP send failed";
        ClosePeer(true);
        return;
    }
    mWriteBuffer.clear();
    mWriteOffset = 0;
}
