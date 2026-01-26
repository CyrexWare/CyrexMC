#include "raknet_peer.hpp"

#include <stdexcept>

cyrex::RakNetPeer::RakNetPeer(RakNet::RakPeerInterface* const peerInterface) : m_peerInterface(peerInterface)
{
    if (m_peerInterface == nullptr)
    {
        throw std::runtime_error("internal error: RakNetPeer was initialized with a nullptr");
    }
}

cyrex::RakNetPeer::StartupResult cyrex::RakNetPeer::startup(const StartupInfo startupInfo)
{
    const auto result = m_peerInterface->Startup(startupInfo.maxConnections,
                                                 startupInfo.socketDescriptors,
                                                 startupInfo.numDescriptors);
    return result;
}

void cyrex::RakNetPeer::shutdown(const ShutdownInfo shutdownInfo)
{
    m_peerInterface->Shutdown(shutdownInfo.blockDuration,
                              shutdownInfo.orderingChannel,
                              shutdownInfo.disconnectionNotificationPriority);
}

void cyrex::RakNetPeer::setMaximumIncomingConnections(const std::uint16_t maxIncomingConnections)
{
    m_peerInterface->SetMaximumIncomingConnections(maxIncomingConnections);
}

cyrex::RakNetPeer::Packet* cyrex::RakNetPeer::receive()
{
    return m_peerInterface->Receive();
}

std::uint32_t cyrex::RakNetPeer::send(const char* data, const int length, const SendInfo sendInfo)
{
    return m_peerInterface->Send(data,
                                 length,
                                 sendInfo.priority,
                                 sendInfo.reliability,
                                 sendInfo.orderingChannel,
                                 sendInfo.systemIdentifier,
                                 sendInfo.broadcast,
                                 sendInfo.forceReceiptNumber);
}

void cyrex::RakNetPeer::deallocatePacket(Packet* packet)
{
}

bool cyrex::RakNetPeer::isActive() const
{
    return m_peerInterface->IsActive();
}
