#include "raknet_connections.hpp"

#include "log/logging.hpp"
#include "network/mcbe/compression/zlib_compressor.hpp"
#include "network/raknet/handler/raknet_handler.hpp"
#include "network/session/network_session.hpp"

#include <ranges>

namespace cyrex::nw::raknet
{
namespace ses = cyrex::nw::session;

void RaknetConnections::onConnect(const RakNet::RakNetGUID& guid, RakNet::SystemAddress address, RaknetHandler* handler)
{
    auto session = std::make_unique<ses::NetworkSession>(guid, address, handler->transport());
    m_sessions.emplace(guid, std::move(session));

    cyrex::logging::log(LOG_RAKNET, "New incoming connection");
}

void RaknetConnections::onDisconnect(const RakNet::RakNetGUID& guid)
{
    auto it = m_sessions.find(guid);
    if (it == m_sessions.end())
        return;

    it->second->markedForDisconnect = true;
    cyrex::logging::log(LOG_RAKNET, "Client disconnected");
}

void RaknetConnections::tick()
{
    for (const auto& session : m_sessions | std::views::values)
    {
        session->tick();
    }
}

void RaknetConnections::cleanup()
{
    for (auto it = m_sessions.begin(); it != m_sessions.end();)
    {
        if (it->second->markedForDisconnect)
            it = m_sessions.erase(it);
        else
            ++it;
    }
}

ses::NetworkSession* RaknetConnections::get(const RakNet::RakNetGUID& guid)
{
    auto it = m_sessions.find(guid);
    if (it == m_sessions.end())
        return nullptr;

    return it->second.get();
}

} // namespace cyrex::nw::raknet
