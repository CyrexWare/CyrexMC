#pragma once

#include "network/mcbe/transport.hpp"
#include "network/raknet/connection/raknet_connections.hpp"
#include "network/raknet/connection/raknet_peer.hpp"

#include <memory>

namespace cyrex
{
class Server;
}

namespace cyrex::nw::raknet
{

class RaknetHandler
{
public:
    explicit RaknetHandler(Server& server);
    ~RaknetHandler();

    void poll();
    proto::Transport* transport() const;
    Server& getServer() const;

private:
    void handlePacket(RakNet::Packet* packet);

    Server& m_server;
    std::unique_ptr<RaknetPeer> m_peer;
    RaknetConnections m_connections;
    std::unique_ptr<proto::Transport> m_transportImpl;
};

} // namespace cyrex::nw::raknet
