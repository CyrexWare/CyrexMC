#pragma once

#include "log/logging.hpp"
// #include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::nw::protocol
{


class DisconnectPacket final :
    public PacketImpl<DisconnectPacket, std::to_underlying(PacketId::Disconnect), PacketDirection::Clientbound, true>
{
public:
    std::int32_t reason = 0;
    std::string message;
    std::string filteredMessage;

    bool decodePayload(io::BinaryReader& in) override
    {
        reason = in.readVarInt();
        if (!in.readBool())
        {
            message = in.readString();
            filteredMessage = in.readString();
        }
        return true;
    }

    bool encodePayload(io::BinaryWriter& out) const override
    {
        out.writeVarInt(reason);
        const bool skip = message.empty() && filteredMessage.empty();
        out.writeBool(skip);
        if (!skip)
        {
            out.writeString(message);
            out.writeString(filteredMessage);
        }
        return true;
    }

    bool handle(session::NetworkSession& /*session*/) override
    {
        return true;
    }
};
} // namespace cyrex::nw::protocol
