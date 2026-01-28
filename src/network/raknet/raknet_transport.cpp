#include "raknet_transport.hpp"
#include <iostream>

void cyrex::network::raknet::RaknetTransport::send(const RakNet::RakNetGUID& guid, const uint8_t* data, size_t len)
{
    peer->Send(reinterpret_cast<const char*>(data), len, HIGH_PRIORITY, RELIABLE_ORDERED, 0, guid, false);
}
