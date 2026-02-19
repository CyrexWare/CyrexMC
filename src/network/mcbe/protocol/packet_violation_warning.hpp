#pragma once

#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/session/network_session.hpp"

#include <cstdint>

namespace cyrex::nw::protocol
{


enum class ViolationSeverity : int32_t
{
    Warning,
    FinalWarning,
    TerminatingConnection
};

class PacketViolationWarningPacket final :
    public PacketImpl<PacketViolationWarningPacket, std::to_underlying(PacketId::PacketViolationWarning), PacketDirection::Serverbound, true>
{
public:
    std::int32_t type{};
    ViolationSeverity severity{};
    std::int32_t packetId{};
    std::string message;

    bool decodePayload(io::BinaryReader& in) override
    {
        type = in.readVarInt();
        severity = static_cast<ViolationSeverity>(in.readVarInt());
        packetId = in.readVarInt();
        message = in.readString();
        return true;
    }

    bool encodePayload(io::BinaryWriter&) const override
    {
        // NOOP
        return false;
    }

    bool handle(session::NetworkSession& session) override
    {
        // session.
        // TODO: handle
        return true;
    }
};

} // namespace cyrex::nw::protocol
