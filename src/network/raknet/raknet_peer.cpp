#include "raknet_peer.hpp"

cyrex::network::raknet::RaknetPeer::RaknetPeer(uint16_t port, uint32_t maxPlayers)
{
    peer = RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor sd(port, nullptr);
    peer->Startup(maxPlayers, &sd, 1);
    peer->SetMaximumIncomingConnections(maxPlayers);
}

cyrex::network::raknet::RaknetPeer::~RaknetPeer()
{
    RakNet::RakPeerInterface::DestroyInstance(peer);
}

RakNet::RakPeerInterface* cyrex::network::raknet::RaknetPeer::get()
{
    return peer;
}
