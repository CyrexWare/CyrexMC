#include "network/mcbe/packet_pool.hpp"

#include "network/mcbe/protocol/protocol_info.hpp"
#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"

void cyrex::network::mcbe::PacketPool::registerAll()
{
    bind(
        cyrex::network::mcbe::protocol::ProtocolInfo::requestNetworkSettingsPacket,
        [] { return std::make_unique<cyrex::network::mcbe::protocol::RequestNetworkSettingsPacket>(); },
        PacketDirection::Serverbound);
    bind(
        cyrex::network::mcbe::protocol::ProtocolInfo::networkSettingsPacket,
        [] { return std::make_unique<cyrex::network::mcbe::protocol::NetworkSettingsPacket>(); },
        PacketDirection::Clientbound);
    bind(
        cyrex::network::mcbe::protocol::ProtocolInfo::loginPacket,
        [] { return std::make_unique<cyrex::network::mcbe::protocol::LoginPacket>(); },
        PacketDirection::Serverbound);
    bind(
        cyrex::network::mcbe::protocol::ProtocolInfo::playStatusPacket,
        [] { return std::make_unique<cyrex::network::mcbe::protocol::PlayStatusPacket>(); },
        PacketDirection::Clientbound);
}
