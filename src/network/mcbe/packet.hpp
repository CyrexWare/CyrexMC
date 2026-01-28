#pragma once

#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/session/network_session.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "packetbase.hpp"

namespace cyrex::network::mcbe
{

class Packet : public PacketBase
{
public:
    void decode(cyrex::network::io::BinaryReader& in) final
    {
        decodePayload(in);
    }

    void encode(cyrex::network::io::BinaryWriter& out) const final
    {
        out.buffer.clear();

        cyrex::network::io::BinaryWriter payload;
        encodePayload(payload);

        out.writeI8(0xFE); //still wrong but yeah this is fine for testing
        out.writeVarInt(payload.length() + 1); // + packetId
        out.writeVarInt(networkId());

        out.writeBuffer(payload.data(), payload.length());
    }


protected:
    virtual void decodePayload(cyrex::network::io::BinaryReader& in) = 0;
    virtual void encodePayload(cyrex::network::io::BinaryWriter& out) const = 0;
};
} // namespace cyrex::network::mcbe
