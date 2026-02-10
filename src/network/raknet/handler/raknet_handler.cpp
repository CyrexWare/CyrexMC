#include "raknet_handler.hpp"

#include "log/logging.hpp"
#include "mcbe_packet_router.hpp"
#include "network/raknet/connection/raknet_transport.hpp"
#include "network/raknet/motd/raknet_motd.hpp"
#include "network/session/network_session.hpp"
#include "server.hpp"

#include <RakNet/MessageIdentifiers.h>
#include <string>

namespace cyrex::nw::raknet
{

RaknetHandler::RaknetHandler(cyrex::Server& server) : m_server(server)
{
    m_peer = std::make_unique<RaknetPeer>();

    INetworkPeer::StartupInfo info{};
    info.port = static_cast<std::uint16_t>(m_server.getPort());
    info.maxConnections = static_cast<std::uint32_t>(m_server.getMaxPlayers());
    // What todo with the returned value?
    m_peer->startup(info);

    m_peer->setMaximumIncomingConnections(static_cast<std::uint16_t>(m_server.getMaxPlayers()));

    const std::string motd = buildMotd(m_server);
    std::string response;
    const auto len = static_cast<uint16_t>(motd.size());
    response.push_back((len >> 8) & 0xFF);
    response.push_back(len & 0xFF);
    response += motd;

    m_peer->get()->SetOfflinePingResponse(response.c_str(), response.size());

    m_transportImpl = std::make_unique<RaknetTransport>(m_peer->get());

    cyrex::logging::log(LOG_RAKNET, "Server began to listen on {} [IPv4]", m_server.getPort());
}

RaknetHandler::~RaknetHandler() = default;

cyrex::nw::protocol::Transport* RaknetHandler::transport() const
{
    return m_transportImpl.get();
}

cyrex::Server& RaknetHandler::getServer() const
{
    return m_server;
}

void RaknetHandler::poll()
{
    INetworkPeer* peer = m_peer.get();

    for (Packet* packet = peer->receive(); packet; peer->deallocatePacket(packet), packet = peer->receive())
    {
        handlePacket(packet);
    }

    m_connections.tick();
    m_connections.cleanup();
}

void RaknetHandler::handlePacket(Packet* packet)
{
    if (!packet || !packet->data || packet->length == 0)
        return;

    switch (packet->data[0])
    {
        case ID_NEW_INCOMING_CONNECTION:
            m_connections.onConnect(packet->guid, packet->systemAddress, this, getServer());
            break;

        case ID_CONNECTION_LOST:
        case ID_DISCONNECTION_NOTIFICATION:
            m_connections.onDisconnect(packet->guid);
            break;

        case 0xFE:
            McbePacketRouter::route(packet, m_connections);
            break;

        default:
            break;
    }
}

} // namespace cyrex::nw::raknet
