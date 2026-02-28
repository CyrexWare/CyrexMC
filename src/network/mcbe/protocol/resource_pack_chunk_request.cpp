#include "resource_pack_chunk_request.hpp"

#include "network/session/network_session.hpp"
#include "player/player.hpp"

bool cyrex::nw::protocol::ResourcePackChunkRequestPacket::handle(session::NetworkSession& session)
{
    return session.getPlayer(subClientId)->handleResourcePackChunkRequest(*this);
}
