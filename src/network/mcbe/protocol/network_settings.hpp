#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

#include <cstdint>

using namespace cyrex::network::io;
using namespace cyrex::network::session;

namespace cyrex::network::mcbe::protocol
{
class NetworkSettingsPacket final : public cyrex::network::mcbe::Packet
{
public:
    static constexpr uint16_t COMPRESS_NOTHING = 0;
    static constexpr uint16_t COMPRESS_EVERYTHING = 1;

    int8_t compressionThreshold = 1;
    int8_t compressionAlgorithm = 0;

    int16_t padding = 0;

    bool enableClientThrottling = false;
    int8_t clientThrottleThreshold = 0;
    float clientThrottleScalar = 0.0f;

    int8_t trailingZero = 0;

    uint32_t networkId() const override
    {
        return ProtocolInfo::NETWORK_SETTINGS_PACKET;
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
    void decodePayload(cyrex::network::io::BinaryReader&) override
    {
        // NOOP
    }

    void encodePayload(cyrex::network::io::BinaryWriter& out) const override
    {
        out.writeI8(compressionThreshold);
        out.writeI8(compressionAlgorithm);
        out.writeI16LE(padding);
        out.writeBool(enableClientThrottling);
        out.writeI8(clientThrottleThreshold);
        out.writeFloatLE(clientThrottleScalar);
        out.writeI8(trailingZero);
    }

public:
    bool handle(cyrex::network::session::NetworkSession&) override
    {
        return true;
    }
};
} // namespace cyrex::network::mcbe::protocol
