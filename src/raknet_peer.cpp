#include "raknet_peer.hpp"

// Duplicate raknet peer, we will need to implement this in our other one

#include <stdexcept>

cyrex::RakNetPeer::RakNetPeer(RakNet::RakPeerInterface* peerInterface) : m_peerInterface(peerInterface)
{
    if (m_peerInterface == nullptr)
    {
        throw std::runtime_error("internal error: RakNetPeer was initialized with a nullptr");
    }
}

cyrex::StartupResult cyrex::RakNetPeer::startup(StartupInfo startupInfo)
{
    RakNet::SocketDescriptor socketDescriptor(startupInfo.port, nullptr);
    const auto result = m_peerInterface->Startup(startupInfo.maxConnections, &socketDescriptor, 1);
    return result;
}

void cyrex::RakNetPeer::shutdown(const ShutdownInfo shutdownInfo)
{
    m_peerInterface->Shutdown(shutdownInfo.blockDuration,
                              shutdownInfo.orderingChannel,
                              shutdownInfo.disconnectionNotificationPriority);
}

void cyrex::RakNetPeer::setMaximumIncomingConnections(std::uint16_t maxIncomingConnections)
{
    m_peerInterface->SetMaximumIncomingConnections(maxIncomingConnections);
}

cyrex::Packet* cyrex::RakNetPeer::receive()
{
    return m_peerInterface->Receive();
}

std::uint32_t cyrex::RakNetPeer::send(const char* data, std::uint32_t length, SendInfo sendInfo)
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
