#include "network/mcbe/packet_factory.hpp"

#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"


void cyrex::network::mcbe::PacketFactory::registerAll()
{
    add<protocol::RequestNetworkSettingsPacket>();
    add<protocol::NetworkSettingsPacket>();
    add<protocol::LoginPacket>();
    add<protocol::PlayStatusPacket>();
}
