#pragma once

#include "log/logging.hpp"
// #include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"
#include "types/PlayStatus.hpp"

namespace cyrex::nw::protocol
{
class PlayStatusPacket final :
    public PacketImpl<PlayStatusPacket, std::to_underlying(PacketId::PlayStatus), PacketDirection::Clientbound, true>
{
public:
    PlayStatus status = PlayStatus::LoginSuccess;

    bool decodePayload(io::BinaryReader& in) override
    {
        status = static_cast<PlayStatus>(in.readU32BE());
        return true;
    }

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeU32BE(static_cast<uint32_t>(status));
        return true;
    }

    bool handle(session::NetworkSession& /*session*/) override
    {
        return true;
    }
};
} // namespace cyrex::nw::protocol
