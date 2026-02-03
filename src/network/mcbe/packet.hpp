#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

namespace cyrex::network::session
{
class NetworkSession;
}

namespace cyrex::network::mcbe
{

class PacketDef;

class Packet
{
public:
    virtual ~Packet() = default;
    Packet(const PacketDef& def) : m_def{def}
    {
    }

    [[nodiscard]] const PacketDef& getDef() const
    {
        return m_def;
    }

    bool decode(io::BinaryReader& in)
    {
        return decodePayload(in);
    }

    bool encode(io::BinaryWriter& out) const;

    virtual bool decodePayload(io::BinaryReader& in) = 0;
    virtual bool encodePayload(io::BinaryWriter& out) const = 0;

    virtual bool handle(session::NetworkSession& session) = 0;

private:
    const PacketDef& m_def;
};
} // namespace cyrex::network::mcbe
