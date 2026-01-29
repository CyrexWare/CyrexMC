#include "packet.hpp"
#include "packet_def.hpp"

bool cyrex::network::mcbe::Packet::encode(cyrex::network::io::BinaryWriter& out) const
{
    cyrex::network::io::BinaryWriter payload;
    if (!encodePayload(payload))
    {
        return false;
    }

    out.writeVarUInt(out.length() + 1);
    out.writeVarUInt(def.networkId);
    out.writeBuffer(payload.data(), payload.length());

    return true;
}