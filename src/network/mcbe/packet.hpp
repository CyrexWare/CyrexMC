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

    [[nodiscard]] [[nodiscard]] [[nodiscard]] const PacketDef& getDef() const
    {
        return m_def;
    }

    bool decode(cyrex::network::io::BinaryReader& in)
    {
        return decodePayload(in);
    }

    bool encode(cyrex::network::io::BinaryWriter& out) const;

    virtual bool decodePayload(cyrex::network::io::BinaryReader& in) = 0;
    virtual bool encodePayload(cyrex::network::io::BinaryWriter& out) const = 0;

    virtual bool handle(cyrex::network::session::NetworkSession& session) = 0;

private:
    const PacketDef& m_def;
};
} // namespace cyrex::network::mcbe
