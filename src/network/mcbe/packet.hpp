#pragma once

#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/session/network_session.hpp"
#include "network/util/binary_stream.hpp"
#include "packetbase.hpp"

using namespace cyrex::network::util;

namespace cyrex::network::mcbe
{

class Packet : public PacketBase
{
public:
    void decode(BinaryStream& in) final
    {
        decodePayload(in);
    }

    void encode(BinaryStream& out) const final
    {
        out.buffer.clear();
        out.offset = 0;

        BinaryStream payload;
        encodePayload(payload);

        out.writeU8(0xFE);
        out.writeI8(static_cast<int8_t>(payload.length() + 1)); // + packetId
        out.writeU8(static_cast<uint8_t>(networkId()));

        out.writeBuffer(payload.data(), payload.length());
    }


protected:
    virtual void decodePayload(BinaryStream& in) = 0;
    virtual void encodePayload(BinaryStream& out) const = 0;
};
} // namespace cyrex::network::mcbe