#pragma once
#include <RakNet/RakPeerInterface.h>

namespace cyrex::network::raknet
{
class RaknetPeer
{
public:
    RaknetPeer(uint16_t port, uint32_t maxPlayers);
    ~RaknetPeer();

    RakNet::RakPeerInterface* get();

private:
    RakNet::RakPeerInterface* peer;
};
} // namespace cyrex::network::raknet