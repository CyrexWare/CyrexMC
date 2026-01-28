#include "raknet_peer.hpp"

cyrex::network::raknet::RaknetPeer::RaknetPeer(uint16_t port, uint32_t maxPlayers)
{
    m_peer = RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor sd(port, nullptr);
    m_peer->Startup(maxPlayers, &sd, 1);
    m_peer->SetMaximumIncomingConnections(maxPlayers);
}

cyrex::network::raknet::RaknetPeer::~RaknetPeer()
{
    RakNet::RakPeerInterface::DestroyInstance(m_peer);
}

RakNet::RakPeerInterface* cyrex::network::raknet::RaknetPeer::get()
{
    return m_peer;
}
