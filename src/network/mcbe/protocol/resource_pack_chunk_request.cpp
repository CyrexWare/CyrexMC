#include "resource_pack_chunk_request.hpp"

#include "network/session/network_session.hpp"

bool cyrex::nw::protocol::ResourcePackChunkRequestPacket::handle(cyrex::nw::session::NetworkSession& session)
{
    return session.handleResourcePackChunkRequest(*this);
}
