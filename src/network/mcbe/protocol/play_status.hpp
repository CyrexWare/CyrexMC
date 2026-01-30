#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

#include <cstdint>

namespace cyrex::network::mcbe::protocol
{
class PlayStatusPacket final : public cyrex::network::mcbe::Packet
{
public:
    using Packet::Packet;

    // Mabye make this separately?
    static constexpr uint32_t loginSuccess = 0;
    static constexpr uint32_t loginFailedClient = 1;
    static constexpr uint32_t loginFailedServer = 2;
    static constexpr uint32_t playerSpawn = 3;
    static constexpr uint32_t loginFailedInvalidTenant = 4;
    static constexpr uint32_t loginFailedVanillaEdu = 5;
    static constexpr uint32_t loginFailedEduVanilla = 6;
    static constexpr uint32_t loginFailedServerFull = 7;
    static constexpr uint32_t loginFailedEditorVanilla = 8;
    static constexpr uint32_t loginFailedVanillaEditor = 9;

    uint32_t status = 0;

protected:
    bool decodePayload(cyrex::network::io::BinaryReader& in) override
    {
        status = in.readU32BE();
        return false;
    }

    bool encodePayload(cyrex::network::io::BinaryWriter& out) const override
    {
        out.writeU32BE(status);
        return false;
    }

public:
    bool handle(cyrex::network::session::NetworkSession& /*session*/) override
    {
        return true;
    }
};

class PlayStatusPacketDef final : public cyrex::network::mcbe::PacketDefImpl<PlayStatusPacket>
{
public:
    PlayStatusPacketDef() :
        PacketDefImpl{ProtocolInfo::playStatusPacket, cyrex::network::mcbe::PacketDirection::Clientbound, true}
    {
    }
};
} // namespace cyrex::network::mcbe::protocol
