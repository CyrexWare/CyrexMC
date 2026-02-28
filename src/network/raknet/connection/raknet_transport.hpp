#pragma once

#include "network/mcbe/transport.hpp"

#include <RakNet/RakNetTypes.h>
#include <RakNet/RakPeerInterface.h>

namespace cyrex::network::raknet
{

class RaknetTransport : public cyrex::network::protocol::Transport
{
public:
    explicit RaknetTransport(RakNet::RakPeerInterface* peer) : m_peer(peer)
    {
    }

    void send(const RakNet::RakNetGUID& guid, const uint8_t* data, size_t len) override;

private:
    RakNet::RakPeerInterface* m_peer = nullptr;
};
} // namespace cyrex::network::raknet
