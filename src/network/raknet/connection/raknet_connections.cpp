#include "raknet_connections.hpp"

#include "log/logging.hpp"
#include "network/mcbe/compression/zlib_compressor.hpp"
#include "network/raknet/handler/raknet_handler.hpp"
#include "network/session/network_session.hpp"

#include <iostream>
#include <ranges>

using namespace cyrex::util;

void cyrex::network::raknet::RaknetConnections::onConnect(
    const RakNet::RakNetGUID& guid,
    RakNet::SystemAddress address,
    cyrex::network::raknet::RaknetHandler* handler,
    cyrex::Server& server)
{
    auto session = std::make_unique<cyrex::network::session::NetworkSession>(guid, address, handler->transport(), server);

    m_sessions.emplace(guid, std::move(session));

    cyrex::logging::log(LOG_RAKNET, "New incoming connection");
}

void cyrex::network::raknet::RaknetConnections::onDisconnect(const RakNet::RakNetGUID& guid)
{
    auto it = m_sessions.find(guid);
    if (it == m_sessions.end())
        return;

    it->second->markedForDisconnect = true;
    cyrex::logging::log(LOG_RAKNET, "Client disconnected");
}

void cyrex::network::raknet::RaknetConnections::tick()
{
    for (const auto& session : m_sessions | std::views::values)
    {
        session->tick();
    }
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
