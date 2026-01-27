#pragma once
#include "network_peer.hpp"

#include <RakNet/RakPeerInterface.h>

namespace cyrex
{

class RakNetPeer : public INetworkPeer
{
public:
    explicit RakNetPeer(RakNet::RakPeerInterface* peerInterface);
    [[nodiscard]] StartupResult startup(StartupInfo startupInfo) override;
    void shutdown(ShutdownInfo shutdownInfo) override;
    void setMaximumIncomingConnections(std::uint16_t maxIncomingConnections) override;
    [[nodiscard]] Packet* receive() override;
    [[nodiscard]] std::uint32_t send(const char* data, std::uint32_t length, SendInfo sendInfo) override;
    void deallocatePacket(Packet* packet) override;
    [[nodiscard]] bool isActive() const override;

private:
    RakNet::RakPeerInterface* const m_peerInterface{};
};

} // namespace cyrex