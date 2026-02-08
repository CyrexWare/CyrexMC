#pragma once

#include "network/io/binary_reader.hpp"
#include "packet_direction.hpp"

#include <memory>

#include <cstdint>

namespace cyrex::nw::session
{
class NetworkSession;
}

namespace cyrex::nw::protocol
{
namespace io = cyrex::nw::io;

class Packet;

class PacketDef
{
public:
    using CreateFunc = std::unique_ptr<Packet> (*)();

    const uint32_t networkId;
    const PacketDirection direction;
    const bool allowBeforeLogin;
    const CreateFunc create;

    constexpr PacketDef(uint32_t networkId, PacketDirection direction, bool allowBeforeLogin, CreateFunc create) :
        networkId{networkId},
        direction{direction},
        allowBeforeLogin{allowBeforeLogin},
        create{create}
    {
    }

    std::unique_ptr<Packet> decode(io::BinaryReader& in) const;
};

} // namespace cyrex::nw::protocol
