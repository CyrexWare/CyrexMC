#pragma once

#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"

#include <cstdint>

using namespace cyrex::network::io;
using namespace cyrex::network::session;

namespace cyrex::network::mcbe::protocol
{
class PlayStatusPacket final : public cyrex::network::mcbe::Packet
{
public:
    // Mabye make this separately?
    static constexpr uint32_t LOGIN_SUCCESS = 0;
    static constexpr uint32_t LOGIN_FAILED_CLIENT = 1;
    static constexpr uint32_t LOGIN_FAILED_SERVER = 2;
    static constexpr uint32_t PLAYER_SPAWN = 3;
    static constexpr uint32_t LOGIN_FAILED_INVALID_TENANT = 4;
    static constexpr uint32_t LOGIN_FAILED_VANILLA_EDU = 5;
    static constexpr uint32_t LOGIN_FAILED_EDU_VANILLA = 6;
    static constexpr uint32_t LOGIN_FAILED_SERVER_FULL = 7;
    static constexpr uint32_t LOGIN_FAILED_EDITOR_VANILLA = 8;
    static constexpr uint32_t LOGIN_FAILED_VANILLA_EDITOR = 9;

    uint32_t status = 0;

    uint32_t networkId() const override
    {
        return ProtocolInfo::PLAY_STATUS_PACKET;
    }

    cyrex::network::mcbe::PacketDirection direction() const override
    {
        return cyrex::network::mcbe::PacketDirection::Clientbound;
    }

    bool allowBeforeLogin() const override
    {
        return true;
    }

protected:
    void decodePayload(BinaryReader& in) override
    {
        status = in.readU32BE();
    }

    void encodePayload(BinaryWriter& out) const override
    {
        out.writeU32BE(status);
    }

public:
    bool handle(NetworkSession& /*session*/) override
    {
        return true;
    }
};
} // namespace cyrex::network::mcbe::protocol
