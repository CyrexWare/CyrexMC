#include "network/mcbe/packet_factory.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"

#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"

void cyrex::network::mcbe::PacketFactory::registerAll()
{
    add<cyrex::network::mcbe::protocol::RequestNetworkSettingsPacketDef>();
    add<cyrex::network::mcbe::protocol::NetworkSettingsPacketDef>();
    add<cyrex::network::mcbe::protocol::LoginPacketDef>();
    add<cyrex::network::mcbe::protocol::PlayStatusPacketDef>();
}
