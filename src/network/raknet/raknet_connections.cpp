#include "raknet_connections.hpp"

#include "network/mcbe/compression/zlib_compressor.hpp"
#include "network/session/network_session.hpp"
#include "raknet_handler.hpp"
#include "util/textformat.hpp"

#include <iostream>

using namespace cyrex::util;

void cyrex::network::raknet::RaknetConnections::onConnect(const RakNet::RakNetGUID& guid,
                                                          RakNet::SystemAddress address,
                                                          cyrex::network::raknet::RaknetHandler* handler)
{
    auto session = std::make_unique<cyrex::network::session::NetworkSession>(guid, address, handler->transport());

    session->setCompressor(
        std::make_unique<cyrex::network::mcbe::compression::ZlibCompressor>(6, std::optional<size_t>{0}, 2 * 1024 * 1024));

    m_sessions.emplace(guid, std::move(session));

    std::cout << renderConsole(bedrock(Color::RED) + "[RAKNET] " + bedrock(Color::DARK_GRAY) + "New incoming connection", true)
              << std::endl;
}

void cyrex::network::raknet::RaknetConnections::onDisconnect(const RakNet::RakNetGUID& guid)
{
    auto it = m_sessions.find(guid);
    if (it == m_sessions.end())
        return;

    it->second->markedForDisconnect = true;
    std::cout << renderConsole(bedrock(Color::RED) + "[RAKNET] " + bedrock(Color::DARK_GRAY) + "Client disconnected", true)
              << std::endl;
}

void cyrex::network::raknet::RaknetConnections::cleanup()
{
    for (auto it = m_sessions.begin(); it != m_sessions.end();)
    {
        if (it->second->markedForDisconnect)
            it = m_sessions.erase(it);
        else
            ++it;
    }
}


cyrex::network::session::NetworkSession* cyrex::network::raknet::RaknetConnections::get(const RakNet::RakNetGUID& guid)
{
    auto it = m_sessions.find(guid);
    if (it == m_sessions.end())
        return nullptr;

    return it->second.get();
}
