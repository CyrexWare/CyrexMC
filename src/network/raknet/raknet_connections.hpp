#pragma once

#include "raknet_guid_hash.hpp"

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
    void onConnect(const RakNet::RakNetGUID& guid, RakNet::SystemAddress address, RaknetHandler* handler);

    void onDisconnect(const RakNet::RakNetGUID& guid);
    void cleanup();

    cyrex::network::session::NetworkSession* get(const RakNet::RakNetGUID& guid);

private:
    std::unordered_map<RakNet::RakNetGUID, std::unique_ptr<cyrex::network::session::NetworkSession>> sessions;
};
} // namespace cyrex::network::raknet
