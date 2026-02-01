#include "packet.hpp"

#include "packet_def.hpp"

bool cyrex::network::mcbe::Packet::encode(cyrex::network::io::BinaryWriter& out) const
{
    cyrex::network::io::BinaryWriter payload;
    payload.buffer.clear();
    if (!encodePayload(payload))
    {
        return false;
    }

    out.writeU8(0xFE);
    out.writeVarUInt(payload.length() + 1);
    out.writeU8(m_def.networkId);
    out.writeBuffer(payload.data(), payload.length());

    return true;
}