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
    using Packet::Packet;

    static constexpr uint16_t compressNothing = 0;
    static constexpr uint16_t compressEverything = 1;

    int16_t compressionThreshold = 1;
    int16_t compressionAlgorithm = 0;

    bool enableClientThrottling = false;
    int8_t clientThrottleThreshold = 0;
    float clientThrottleScalar = 0.0f;

protected:
    bool decodePayload(cyrex::network::io::BinaryReader&) override
    {
        // NOOP
        return false;
    }

    bool encodePayload(cyrex::network::io::BinaryWriter& out) const override
    {
        out.writeI16LE(compressionThreshold);
        out.writeI16LE(compressionAlgorithm);
        out.writeBool(enableClientThrottling);
        out.writeI8(clientThrottleThreshold);
        out.writeFloatLE(clientThrottleScalar);
        return true;
    }

public:
    bool handle(cyrex::network::session::NetworkSession&) override
    {
        return true;
    }
};

class NetworkSettingsPacketDef final : public cyrex::network::mcbe::PacketDefImpl<NetworkSettingsPacket>
{
public:
    NetworkSettingsPacketDef() :
        PacketDefImpl{ProtocolInfo::networkSettingsPacket, cyrex::network::mcbe::PacketDirection::Clientbound, true}
    {
    }
};
} // namespace cyrex::network::mcbe::protocol
