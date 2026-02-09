#include "raknet_peer.hpp"

namespace cyrex::nw::raknet
{

StartupResult RaknetPeer::startup(StartupInfo startupInfo)
{
    if (!m_peer)
        m_peer.reset(RakNet::RakPeerInterface::GetInstance());

    RakNet::SocketDescriptor sd(startupInfo.port, nullptr);
    return m_peer->Startup(startupInfo.maxConnections, &sd, 1);
}

void RaknetPeer::shutdown(ShutdownInfo shutdownInfo)
{
    if (!m_peer)
        return;

    m_peer->Shutdown(static_cast<unsigned int>(shutdownInfo.blockDuration));
}

void RaknetPeer::setMaximumIncomingConnections(std::uint16_t maxIncomingConnections)
{
    if (m_peer)
        m_peer->SetMaximumIncomingConnections(maxIncomingConnections);
}

Packet* RaknetPeer::receive()
{
    return m_peer ? m_peer->Receive() : nullptr;
}

std::uint32_t RaknetPeer::send(const char* data, std::uint32_t length, SendInfo sendInfo)
{
    if (!m_peer || !data || length == 0)
        return 0;

    return m_peer->Send(data,
                        length,
                        sendInfo.priority,
                        sendInfo.reliability,
                        sendInfo.orderingChannel,
                        sendInfo.systemIdentifier,
                        sendInfo.broadcast,
                        sendInfo.forceReceiptNumber);
}

void RaknetPeer::deallocatePacket(Packet* packet)
{
    if (m_peer && packet)
        m_peer->DeallocatePacket(packet);
}

bool RaknetPeer::isActive() const
{
    return m_peer != nullptr;
}

RaknetPeer::~RaknetPeer() = default;

} // namespace cyrex::nw::raknet
