#include "resource_pack_client_response.hpp"

#include "network/session/network_session.hpp"

bool cyrex::nw::protocol::ResourcePackClientResponsePacket::handle(session::NetworkSession& session)
{
    return session.handleResourcePackClientResponse(*this);
}
