#include "raknet_connections.hpp"

#include "network/session/network_session.hpp"
#include "raknet_handler.hpp"
#include "util/textformat.hpp"
#include "network/mcbe/compression/zlib_compressor.hpp"

#include <iostream>

using namespace cyrex::util;

void cyrex::network::raknet::RaknetConnections::onConnect(const RakNet::RakNetGUID& guid,
                                                        RakNet::SystemAddress address,
                                                          cyrex::network::raknet::RaknetHandler* handler)
{
    auto session = std::make_unique<cyrex::network::session::NetworkSession>(guid, address, handler->transport());

    session->setCompressor(std::make_unique<cyrex::network::mcbe::compression::ZlibCompressor>(6, std::optional<size_t>{0}, 2 * 1024 * 1024));

    sessions.emplace(guid, std::move(session));

    std::cout << renderConsole(bedrock(Color::RED) + "[RAKNET] " + bedrock(Color::DARK_GRAY) + "New incoming connection", true)
              << std::endl;
}

void cyrex::network::raknet::RaknetConnections::onDisconnect(const RakNet::RakNetGUID& guid)
{
    sessions.erase(guid);
    std::cout << renderConsole(bedrock(Color::RED) + "[RAKNET] " + bedrock(Color::DARK_GRAY) + "Client disconnected", true)
              << std::endl;
}

cyrex::network::session::NetworkSession* cyrex::network::raknet::RaknetConnections::get(const RakNet::RakNetGUID& guid)
{
    auto it = sessions.find(guid);
    if (it == sessions.end())
        return nullptr;

    return it->second.get();
}
