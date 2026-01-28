#pragma once
#include "raknet_connections.hpp"

namespace cyrex::network::raknet
{
class McbePacketRouter
{
public:
    static void route(RakNet::Packet* p, cyrex::network::raknet::RaknetConnections& connections);
};
} // namespace cyrex::network::raknet
