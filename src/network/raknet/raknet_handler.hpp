#pragma once

#include "network/mcbe/transport.hpp"
#include "raknet_connections.hpp"
#include "raknet_peer.hpp"

#include <RakNet/RakNetTypes.h>
#include <memory>

namespace cyrex
{
class Server;
}

namespace cyrex::network::raknet
{

class RaknetHandler
{
public:
    explicit RaknetHandler(cyrex::Server& server);
    ~RaknetHandler();

    void poll();
    cyrex::network::mcbe::Transport* transport();

private:
    void handlePacket(RakNet::Packet* p);

    cyrex::Server& m_server;

    std::unique_ptr<RaknetPeer> m_peer;
    RaknetConnections m_connections;
    std::unique_ptr<cyrex::network::mcbe::Transport> m_transportImpl;
};

} // namespace cyrex::network::raknet
