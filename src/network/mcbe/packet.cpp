#include "packet.hpp"

#include "packet_def.hpp"

#include <cassert>

bool cyrex::network::mcbe::Packet::encode(cyrex::network::io::BinaryWriter& out) const
{
    cyrex::network::io::BinaryWriter payload;
    payload.buffer.clear();
    if (!encodePayload(payload))
    {
        return false;
    }

    out.writeVarUInt(payload.length() + io::BinaryWriter::getVarUIntSize(getDef().networkId));
    out.writeVarUInt(getDef().networkId);
    out.writeBuffer(payload.data(), payload.length());
    return true;
}