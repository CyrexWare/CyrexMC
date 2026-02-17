#pragma once

#include "network/network_peer.hpp"

#include <RakNet/RakNetTypes.h>
#include <RakNet/RakPeerInterface.h>
#include <memory>

namespace cyrex::network::raknet
{

class RaknetPeer : public cyrex::network::INetworkPeer
{
public:
    explicit RaknetPeer() = default;
    RaknetPeer(const RaknetPeer&) = delete;
    RaknetPeer& operator=(const RaknetPeer&) = delete;

    ~RaknetPeer() override;

    [[nodiscard]] StartupResult startup(StartupInfo startupInfo) override;
    void shutdown(ShutdownInfo shutdownInfo) override;
    void setMaximumIncomingConnections(std::uint16_t maxIncomingConnections) override;
    [[nodiscard]] Packet* receive() override;
    [[nodiscard]] std::uint32_t send(const char* data, std::uint32_t length, SendInfo sendInfo) override;
    void deallocatePacket(Packet* packet) override;
    [[nodiscard]] bool isActive() const override;

    [[nodiscard]] RakNet::RakPeerInterface* get() const noexcept
    {
        return m_peer.get();
    }

private:
    struct RakPeerDeleter
    {
        void operator()(RakNet::RakPeerInterface* peer) const
        {
            if (peer)
                RakNet::RakPeerInterface::DestroyInstance(peer);
        }
    };

    std::unique_ptr<RakNet::RakPeerInterface, RakPeerDeleter> m_peer;
};

} // namespace cyrex::network::raknet
