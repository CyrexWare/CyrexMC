#include "network/mcbe/packet_factory.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"


void cyrex::nw::protocol::PacketFactory::registerAll()
{
    add<cyrex::nw::protocol::RequestNetworkSettingsPacket>();
    add<cyrex::nw::protocol::NetworkSettingsPacket>();
    add<cyrex::nw::protocol::LoginPacket>();
    add<cyrex::nw::protocol::PlayStatusPacket>();
}
