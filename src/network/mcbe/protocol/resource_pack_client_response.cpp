#include "resource_pack_client_response.hpp"

#include "network/session/network_session.hpp"
#include "player/player.hpp"

bool cyrex::network::protocol::ResourcePackClientResponsePacket::handle(session::NetworkSession& session)
{
    return session.getPlayer(subClientId)->handleResourcePackClientResponse(*this);
}
