#include "packet_def.hpp"

#include "packet.hpp"

namespace cyrex::nw::protocol
{
namespace io = cyrex::nw::io;

std::unique_ptr<Packet> PacketDef::decode(io::BinaryReader& in) const
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

} // namespace cyrex::nw::protocol