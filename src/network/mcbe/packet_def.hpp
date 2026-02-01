#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "packet.hpp"
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
    virtual ~PacketDef() = default;

    const uint32_t networkId;
    const PacketDirection direction;
    const bool allowBeforeLogin;

    PacketDef(uint32_t networkId, PacketDirection direction, bool allowBeforeLogin) :
        networkId{networkId},
        direction{direction},
        allowBeforeLogin{allowBeforeLogin}
    {
    }

    [[nodiscard]] virtual std::unique_ptr<Packet> create() const = 0;

    std::unique_ptr<Packet> decode(cyrex::network::io::BinaryReader& in) const
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
};

template <typename T>
class PacketDefImpl : public PacketDef
{
public:
    using PacketType = T;

    using PacketDef::PacketDef;

    PacketDefImpl(uint32_t networkId, PacketDirection direction, bool allowBeforeLogin) :
        PacketDef{networkId, direction, allowBeforeLogin}
    {
    }

    [[nodiscard]] std::unique_ptr<Packet> create() const override
    {
        return std::make_unique<T>(*this);
    }
};
} // namespace cyrex::network::mcbe
