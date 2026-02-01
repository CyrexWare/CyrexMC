#include "network/mcbe/packet_factory.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"


void cyrex::network::mcbe::PacketFactory::registerAll()
{
    add<cyrex::network::mcbe::protocol::RequestNetworkSettingsPacket>();
    add<cyrex::network::mcbe::protocol::NetworkSettingsPacket>();
    add<cyrex::network::mcbe::protocol::LoginPacket>();
    add<cyrex::network::mcbe::protocol::PlayStatusPacket>();
}
