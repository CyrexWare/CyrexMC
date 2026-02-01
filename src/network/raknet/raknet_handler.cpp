#include "raknet_handler.hpp"

#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "mcbe_packet_router.hpp"
#include "network/session/network_session.hpp"
#include "raknet_motd.hpp"
#include "raknet_transport.hpp"
#include "server.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"

#include <RakNet/MessageIdentifiers.h>
#include <iostream>
#include <memory>
using namespace cyrex::util;

cyrex::network::raknet::RaknetHandler::RaknetHandler(cyrex::Server& server) : m_server(server)
{
    m_peer = std::make_unique<RaknetPeer>(m_server.getPort(), m_server.getMaxPlayers());

    RakNet::RakPeerInterface* rakPeer = m_peer->get();

    const std::string motd = cyrex::network::raknet::buildRaknetMotd(m_server);
    std::string response;

    const auto len = static_cast<uint16_t>(motd.size());
    response.push_back((len >> 8) & 0xFF);
    response.push_back(len & 0xFF);
    response += motd;

    rakPeer->SetOfflinePingResponse(response.c_str(), response.size());

    m_transportImpl = std::make_unique<RaknetTransport>(rakPeer);
    cyrex::log::sendConsoleMessage(cyrex::log::MessageType::RAKNET_LOG,
                                   cyrex::text::format::Builder()
                                       .color(text::format::Color::DARK_GRAY)
                                       .text("Server began to listen on " + std::to_string(server.getPort()) + " [IPv4]")
                                       .build());
}

cyrex::network::raknet::RaknetHandler::~RaknetHandler() = default;

cyrex::network::mcbe::Transport* cyrex::network::raknet::RaknetHandler::transport()
{
    return m_transportImpl.get();
}

void cyrex::network::raknet::RaknetHandler::poll()
{
    RakNet::RakPeerInterface* rakPeer = m_peer->get();

    for (RakNet::Packet* p = rakPeer->Receive(); p; rakPeer->DeallocatePacket(p), p = rakPeer->Receive())
    {
        handlePacket(p);
    }

    m_connections.tick();
    m_connections.cleanup();
}

void cyrex::network::raknet::RaknetHandler::handlePacket(RakNet::Packet* p)
{
    if (!p || !p->data || p->length == 0)
        return;

    switch (p->data[0])
    {
        case ID_NEW_INCOMING_CONNECTION:
            m_connections.onConnect(p->guid, p->systemAddress, this);
            break;

        case ID_CONNECTION_LOST:
        case ID_DISCONNECTION_NOTIFICATION:
            m_connections.onDisconnect(p->guid);
            break;

        case 0xFE:
            cyrex::network::raknet::McbePacketRouter::route(p, m_connections);
            break;

        default:
            break;
    }
}
