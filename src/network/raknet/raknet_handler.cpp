#include "raknet_handler.hpp"

#include "network/session/network_session.hpp"
#include "raknet_motd.hpp"
#include "raknet_transport.hpp"
#include "server.hpp"
#include "mcbe_packet_router.hpp"
#include "util/textformat.hpp"

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
}

void cyrex::network::raknet::RaknetHandler::handlePacket(RakNet::Packet* p)
{
    const uint8_t id = p->data[0];

    // from switch to this, probably not too big of an impact, but eh
    if (id == 0x13)
        connections.onConnect(p->guid, p->systemAddress, this);
    else if (id == 0x15)
        connections.onDisconnect(p->guid);
    else if (id == 0xFE)
        cyrex::network::raknet::McbePacketRouter::route(p, connections);
}