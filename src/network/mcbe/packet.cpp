#include "packet.hpp"

#include "packet_def.hpp"

#include <cassert>

bool cyrex::network::mcbe::Packet::encode(io::BinaryWriter& out) const
{
    io::BinaryWriter payload;
    payload.buffer.clear();
    if (!encodePayload(payload))
    {
        return false;
    }
    const std::uint32_t header = getDef().networkId & 0x3FF | subClientId << 10;
    out.writeVarUInt(payload.length() + io::BinaryWriter::getVarUIntSize(header));
    out.writeVarUInt(header);
    out.writeBuffer(payload.data(), payload.length());
    return true;
}