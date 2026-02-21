#include "network/mcbe/packet_factory.hpp"

#include "protocol/client_cache_status.hpp"
#include "protocol/client_to_server_handshake.hpp"
#include "protocol/login.hpp"
#include "protocol/network_settings.hpp"
#include "protocol/play_status.hpp"
#include "protocol/request_network_settings.hpp"
#include "protocol/resource_pack_chunk_data.hpp"
#include "protocol/resource_pack_chunk_request.hpp"
#include "protocol/resource_pack_client_response.hpp"
#include "protocol/resource_pack_data_info.hpp"
#include "protocol/resource_pack_stack.hpp"
#include "protocol/resource_packs_info.hpp"
#include "protocol/server_to_client_handshake.hpp"

void cyrex::network::protocol::PacketFactory::registerAll()
{
    add<RequestNetworkSettingsPacket>();
    add<NetworkSettingsPacket>();
    add<LoginPacket>();
    add<PlayStatusPacket>();
    add<ServerToClientHandshakePacket>();
    add<ClientCacheStatusPacket>();
    add<ClientToServerHandshakePacket>();
    add<ResourcePacksInfoPacket>();
    add<ResourcePackChunkDataPacket>();
    add<ResourcePackChunkRequestPacket>();
    add<ResourcePackClientResponsePacket>();
    add<ResourcePackDataInfoPacket>();
    add<ResourcePackStackPacket>();
}
