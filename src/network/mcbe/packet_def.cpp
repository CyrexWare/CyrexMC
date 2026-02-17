#include "packet_def.hpp"

#include "packet.hpp"

std::unique_ptr<cyrex::nw::protocol::Packet> cyrex::nw::protocol::PacketDef::decode(io::BinaryReader& in) const
{
    auto packet = create();
    if (!packet)
    {
        return {};
    }

    if (!packet->decode(in))
    {
        return {};
    }

    return packet;
}
