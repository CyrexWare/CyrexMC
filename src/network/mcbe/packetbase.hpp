#pragma once

#include "network/util/binary_stream.hpp"
#include "packet_direction.hpp"

#include <cstdint>

namespace cyrex::network::session
{
class NetworkSession;
}

namespace cyrex::network::mcbe
{

class PacketBase
{
public:
    virtual ~PacketBase() = default;

    virtual uint32_t networkId() const = 0;
    virtual PacketDirection direction() const = 0;
    virtual bool allowBeforeLogin() const = 0;

    virtual void decode(cyrex::network::util::BinaryStream& in) = 0;
    virtual void encode(cyrex::network::util::BinaryStream& out) const = 0;

    virtual bool handle(cyrex::network::session::NetworkSession& session) = 0;
};
} // namespace cyrex::network::mcbe