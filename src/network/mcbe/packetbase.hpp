#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
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

    [[nodiscard]] virtual uint32_t networkId() const = 0;
    [[nodiscard]] virtual PacketDirection direction() const = 0;
    [[nodiscard]] virtual bool allowBeforeLogin() const = 0;

    virtual void decode(cyrex::network::io::BinaryReader& in) = 0;
    virtual void encode(cyrex::network::io::BinaryWriter& out) const = 0;

    virtual bool handle(cyrex::network::session::NetworkSession& session) = 0;
};
} // namespace cyrex::network::mcbe
