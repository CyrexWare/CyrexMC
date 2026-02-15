#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "packet_def.hpp"
#include "packet_direction.hpp"

namespace cyrex::nw::session
{
class NetworkSession;
}

namespace cyrex::nw::protocol
{

class PacketDef;

class Packet
{
public:
    virtual ~Packet() = default;
    Packet() = default;
    uint8_t subClientId : 3 = 0;

    [[nodiscard]] virtual const PacketDef& getDef() const = 0;

    bool decode(io::BinaryReader& in)
    {
        return decodePayload(in);
    }

    bool encode(io::BinaryWriter& out) const;

    virtual bool decodePayload(io::BinaryReader& in) = 0;
    virtual bool encodePayload(io::BinaryWriter& out) const = 0;

    virtual bool handle(session::NetworkSession& session) = 0;
};

template <typename PacketType, uint32_t networkId, PacketDirection direction, bool allowBeforeLogin>
class PacketImpl : public Packet
{
    friend PacketType;

public:
    [[nodiscard]] static const PacketDef& getDefStatic()
    {
        static const PacketDef def{networkId,
                                   direction,
                                   allowBeforeLogin,
                                   +[]() -> std::unique_ptr<Packet>
                                   {
                                       PacketType* raw = new PacketType();
                                       return std::unique_ptr<Packet>(dynamic_cast<Packet*>(raw));
                                   }};
        return def;
    }

    [[nodiscard]] const PacketDef& getDef() const override
    {
        return getDefStatic();
    }

private:
    PacketImpl() = default;
};
} // namespace cyrex::nw::protocol
