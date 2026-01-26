#pragma once
#include "network_peer.hpp"

#include <RakNet/RakPeerInterface.h>

namespace cyrex
{

class RakNetPeer : public INetworkPeer
{
public:
    explicit RakNetPeer(RakNet::RakPeerInterface* const peerInterface);
    StartupResult startup(const StartupInfo startupInfo) override;
    void shutdown(const ShutdownInfo shutdownInfo) override;
    void setMaximumIncomingConnections(const std::uint16_t maxIncomingConnections) override;
    Packet* receive() override;
    std::uint32_t send(const char* data, const int length, const SendInfo sendInfo) override;
    void deallocatePacket(Packet* packet) override;
    bool isActive() const override;

private:
    RakNet::RakPeerInterface* const m_peerInterface{};
};

} // namespace cyrex