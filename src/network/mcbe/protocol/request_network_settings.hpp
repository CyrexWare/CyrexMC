#pragma once

#include "log/logging.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

#include <cstdint>

namespace cyrex::nw::protocol
{

class RequestNetworkSettingsPacket final :
    public PacketImpl<RequestNetworkSettingsPacket, static_cast<uint32_t>(PacketId::RequestNetworkSettings), PacketDirection::Serverbound, true>
{
public:
    uint32_t protocolVersion = 0;

    bool decodePayload(io::BinaryReader& in) override
    {
        protocolVersion = in.readU32BE();
        cyrex::logging::info(LOG_MCBE, "Received Protocol Version (RequestNetworkSettingsPacket): {}", protocolVersion);
        return true;
    }

    bool encodePayload(io::BinaryWriter&) const override
    {
        // NOOP
        return false;
    }

    bool handle(session::NetworkSession& session) override
    {
        return session.handleRequestNetworkSettings(protocolVersion);
    }
};
} // namespace cyrex::nw::protocol
