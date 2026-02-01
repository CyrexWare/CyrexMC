#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "packet_direction.hpp"

#include <memory>

#include <cstdint>

namespace cyrex::network::session
{
class NetworkSession;
}

namespace cyrex::network::mcbe
{


class PacketDef
{
public:
    using CreateFunc = std::unique_ptr<Packet> (*)();

    const uint32_t networkId;
    const PacketDirection direction;
    const bool allowBeforeLogin;
    CreateFunc create;

    constexpr PacketDef(uint32_t networkId, PacketDirection direction, bool allowBeforeLogin, CreateFunc create) :
        networkId{networkId},
        direction{direction},
        allowBeforeLogin{allowBeforeLogin},
        create{create}
    {
    }

    std::unique_ptr<Packet> decode(cyrex::network::io::BinaryReader& in) const;
};

} // namespace cyrex::network::mcbe
