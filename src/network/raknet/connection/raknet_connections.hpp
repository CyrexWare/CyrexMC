#pragma once

#include "network/raknet/util/raknet_guid_hash.hpp"

#include <RakNet/RakNetTypes.h>
#include <memory>
#include <unordered_map>

namespace cyrex::nw::session
{
class NetworkSession;
}

namespace cyrex::nw::raknet
{
namespace ses = cyrex::nw::session;

class RaknetHandler;

class RaknetConnections
{
public:
    void onConnect(const RakNet::RakNetGUID& guid, RakNet::SystemAddress address, RaknetHandler* handler);
    void onDisconnect(const RakNet::RakNetGUID& guid);
    void cleanup();
    void tick();

    ses::NetworkSession* get(const RakNet::RakNetGUID& guid);

private:
    std::unordered_map<RakNet::RakNetGUID, std::unique_ptr<ses::NetworkSession>> m_sessions;
};

} // namespace cyrex::nw::raknet