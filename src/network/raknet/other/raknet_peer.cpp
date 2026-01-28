#include "raknet_peer.hpp"

// Duplicate raknet peer, we will need to implement this in our other one

#include <stdexcept>

cyrex::network::raknet::other::RakNetPeer::RakNetPeer(RakNet::RakPeerInterface* peerInterface) :
    m_peerInterface(peerInterface)
{
    if (m_peerInterface == nullptr)
    {
        throw std::runtime_error("internal error: RakNetPeer was initialized with a nullptr");
    }
}

cyrex::network::StartupResult cyrex::network::raknet::other::RakNetPeer::startup(StartupInfo startupInfo)
{
    RakNet::SocketDescriptor socketDescriptor(startupInfo.port, nullptr);
    const auto result = m_peerInterface->Startup(startupInfo.maxConnections, &socketDescriptor, 1);
    return result;
}

void cyrex::network::raknet::other::RakNetPeer::shutdown(const ShutdownInfo shutdownInfo)
{
    m_peerInterface->Shutdown(shutdownInfo.blockDuration,
                              shutdownInfo.orderingChannel,
                              shutdownInfo.disconnectionNotificationPriority);
}

void cyrex::network::raknet::other::RakNetPeer::setMaximumIncomingConnections(std::uint16_t maxIncomingConnections)
{
    m_peerInterface->SetMaximumIncomingConnections(maxIncomingConnections);
}

cyrex::network::Packet* cyrex::network::raknet::other::RakNetPeer::receive()
{
    return m_peerInterface->Receive();
}

std::uint32_t cyrex::network::raknet::other::RakNetPeer::send(const char* data, std::uint32_t length, SendInfo sendInfo)
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

void cyrex::network::raknet::other::RakNetPeer::deallocatePacket(cyrex::network::Packet* packet)
{
}

bool cyrex::network::raknet::other::RakNetPeer::isActive() const
{
    return m_peerInterface->IsActive();
}
