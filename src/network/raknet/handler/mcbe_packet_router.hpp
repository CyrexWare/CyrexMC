#pragma once
#include "network/raknet/connection/raknet_connections.hpp"

namespace cyrex::nw::raknet
{
class McbePacketRouter
{
public:
    static void route(RakNet::Packet* p, cyrex::nw::raknet::RaknetConnections& connections);
};
} // namespace cyrex::nw::raknet
