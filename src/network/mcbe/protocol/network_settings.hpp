#pragma once

#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

#include <cstdint>

namespace cyrex::nw::proto
{
class NetworkSettingsPacket final :
    public PacketImpl<NetworkSettingsPacket, std::to_underlying(PacketId::NetworkSettings), PacketDirection::Clientbound, true>
{
public:
    static constexpr uint16_t compressNothing = 0;
    static constexpr uint16_t compressEverything = 1;

    std::int16_t compressionThreshold = 1;
    std::int16_t compressionAlgorithm = 0;

    bool enableClientThrottling = false;
    std::int8_t clientThrottleThreshold = 0;
    float clientThrottleScalar = 0.0f;

    bool decodePayload(io::BinaryReader&) override
    {
        // NOOP
        return false;
    }

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeI16LE(compressionThreshold);
        out.writeI16LE(compressionAlgorithm);
        out.writeBool(enableClientThrottling);
        out.writeI8(clientThrottleThreshold);
        out.writeFloatLE(clientThrottleScalar);
        return true;
    }

    bool handle(session::NetworkSession&) override
    {
        return true;
    }
};

} // namespace cyrex::nw::proto
