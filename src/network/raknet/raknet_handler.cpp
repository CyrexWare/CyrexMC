#include "raknet_handler.hpp"

#include "network/session/network_session.hpp"
#include "raknet_motd.hpp"
#include "raknet_transport.hpp"
#include "server.hpp"
#include "mcbe_packet_router.hpp"
#include "util/textformat.hpp"

#include <RakNet/MessageIdentifiers.h>
#include <iostream>
using namespace cyrex::util;

cyrex::network::raknet::RaknetHandler::RaknetHandler()
{
    cyrex::Server& server = cyrex::Server::getInstance();

    peer = std::make_unique<cyrex::network::raknet::RaknetPeer>(server.getPort(), server.getMaxPlayers());
    RakNet::RakPeerInterface* rakPeer = peer->get();

    std::string motd = cyrex::network::raknet::buildRaknetMotd();
    std::string response;

    uint16_t len = static_cast<uint16_t>(motd.size());
    response.push_back((len >> 8) & 0xFF);
    response.push_back(len & 0xFF);
    response += motd;

    rakPeer->SetOfflinePingResponse(response.c_str(), response.size());

    transportImpl = std::make_unique<cyrex::network::raknet::RaknetTransport>(rakPeer);

    std::cout << renderConsole(bedrock(Color::RED) + "[RAKNET] ", true)
              << renderConsole(bedrock(Color::DARK_GRAY) + "listening on ", false) << server.getPort() << std::endl;
}

cyrex::network::raknet::RaknetHandler::~RaknetHandler() = default;

cyrex::network::mcbe::Transport* cyrex::network::raknet::RaknetHandler::transport()
{
    return transportImpl.get();
}

void cyrex::network::raknet::RaknetHandler::poll()
{
    RakNet::RakPeerInterface* rakPeer = peer->get();

    for (RakNet::Packet* p = rakPeer->Receive(); p; rakPeer->DeallocatePacket(p), p = rakPeer->Receive())
    {
        handlePacket(p);
    }

    connections.cleanup();
}

void cyrex::network::raknet::RaknetHandler::handlePacket(RakNet::Packet* p)
{
    if (!p || !p->data || p->length == 0)
        return;

    switch (p->data[0])
    {
        case ID_NEW_INCOMING_CONNECTION:
            connections.onConnect(p->guid, p->systemAddress, this);
            break;

        case ID_CONNECTION_LOST:
        case ID_DISCONNECTION_NOTIFICATION:
            connections.onDisconnect(p->guid);
            break;

        case 0xFE:
            cyrex::network::raknet::McbePacketRouter::route(p, connections);
            break;

        default:
            break;
    }
}