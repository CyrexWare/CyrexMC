#include "resource_pack_chunk_request.hpp"

#include "network/session/network_session.hpp"

bool cyrex::nw::proto::ResourcePackChunkRequestPacket::handle(session::NetworkSession& session)
{
    return session.handleResourcePackChunkRequest(*this);
}
