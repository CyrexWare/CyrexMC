#pragma once

#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

#include <cstdint>

using namespace cyrex::nw::io;
using namespace cyrex::nw::session;

namespace cyrex::nw::protocol
{
class NetworkSettingsPacket final :
    public cyrex::nw::protocol::PacketImpl<NetworkSettingsPacket,
                                           static_cast<uint32_t>(PacketId::NetworkSettings),
                                           cyrex::nw::protocol::PacketDirection::Clientbound,
                                           true>
{
public:
    static constexpr uint16_t compressNothing = 0;
    static constexpr uint16_t compressEverything = 1;

    std::int16_t compressionThreshold = 1;
    std::int16_t compressionAlgorithm = 0;

    bool enableClientThrottling = false;
    std::int8_t clientThrottleThreshold = 0;
    float clientThrottleScalar = 0.0f;

    bool decodePayload(cyrex::nw::io::BinaryReader&) override
    {
        // NOOP
        return false;
    }

    bool encodePayload(cyrex::nw::io::BinaryWriter& out) const override
    {
        out.writeI16LE(compressionThreshold);
        out.writeI16LE(compressionAlgorithm);
        out.writeBool(enableClientThrottling);
        out.writeI8(clientThrottleThreshold);
        out.writeFloatLE(clientThrottleScalar);
        return true;
    }

    bool handle(cyrex::nw::session::NetworkSession&) override
    {
        return true;
    }
};

} // namespace cyrex::nw::protocol
