#pragma once

#include "network/raknet/util/raknet_guid_hash.hpp"
#include "server.hpp"

#include <RakNet/RakNetTypes.h>
#include <memory>
#include <unordered_map>

namespace cyrex::network::session
{
class NetworkSession;
}

namespace cyrex::network::raknet
{
class RaknetHandler;

class RaknetConnections
{
public:
    void onConnect(const RakNet::RakNetGUID& guid, RakNet::SystemAddress address, RaknetHandler* handler, cyrex::Server& server);

    void onDisconnect(const RakNet::RakNetGUID& guid);
    void cleanup();

    void tick();

    cyrex::network::session::NetworkSession* get(const RakNet::RakNetGUID& guid);

private:
    std::unordered_map<RakNet::RakNetGUID, std::unique_ptr<cyrex::network::session::NetworkSession>> m_sessions;
};
} // namespace cyrex::network::raknet
