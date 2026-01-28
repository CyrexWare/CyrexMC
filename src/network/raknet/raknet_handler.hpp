#pragma once

#include "network/mcbe/transport.hpp"
#include "raknet_connections.hpp"
#include "raknet_peer.hpp"

#include <RakNet/RakNetTypes.h>
#include <memory>

namespace cyrex::network::raknet
{

class RaknetHandler
{
public:
    RaknetHandler();
    ~RaknetHandler();

    void poll();

    cyrex::network::mcbe::Transport* transport();

private:
    void handlePacket(RakNet::Packet* p);

private:
    std::unique_ptr<RaknetPeer> peer;
    RaknetConnections connections;

    std::unique_ptr<cyrex::network::mcbe::Transport> transportImpl;
};

} // namespace cyrex::network::raknet
