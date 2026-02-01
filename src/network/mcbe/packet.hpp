#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "packet_def.hpp"
#include "packet_direction.hpp"

namespace cyrex::network::session
{
class NetworkSession;
}

namespace cyrex::network::mcbe
{

class PacketDef;

class Packet
{
public:
    virtual ~Packet() = default;
    Packet() = default;

    [[nodiscard]] virtual const PacketDef& getDef() const = 0;

    bool decode(cyrex::network::io::BinaryReader& in)
    {
        return decodePayload(in);
    }

    bool encode(cyrex::network::io::BinaryWriter& out) const;

    virtual bool decodePayload(cyrex::network::io::BinaryReader& in) = 0;
    virtual bool encodePayload(cyrex::network::io::BinaryWriter& out) const = 0;

    virtual bool handle(cyrex::network::session::NetworkSession& session) = 0;
};

template <typename PacketType, uint32_t networkId, PacketDirection direction, bool allowBeforeLogin>
class PacketImpl : public Packet
{
public:
    [[nodiscard]] static const PacketDef& getDefStatic()
    {
        static PacketDef def{networkId,
                             direction,
                             allowBeforeLogin,
                             +[]() -> std::unique_ptr<Packet>
                             {
                                 return std::unique_ptr<Packet>(std::make_unique<PacketType>());
                             }};
        return def;
    }

    [[nodiscard]] const PacketDef& getDef() const override
    {
        return getDefStatic();
    }
};
} // namespace cyrex::network::mcbe
