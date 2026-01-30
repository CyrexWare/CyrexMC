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
    Packet(const PacketDef& def) : def{def}
    {
    }

    const PacketDef& getDef() const
    {
        return def;
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
    const PacketDef& def;
};
} // namespace cyrex::network::mcbe
