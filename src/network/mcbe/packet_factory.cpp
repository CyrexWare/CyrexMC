#include "network/mcbe/packet_factory.hpp"

#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"


void cyrex::nw::protocol::PacketFactory::registerAll()
{
    add<RequestNetworkSettingsPacket>();
    add<NetworkSettingsPacket>();
    add<LoginPacket>();
    add<PlayStatusPacket>();
}
