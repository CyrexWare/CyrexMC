#pragma once

#include "log/logging.hpp"
#include "network/session/network_session.hpp"

namespace cyrex::network::mcbe::protocol
{

class RequestNetworkSettingsPacket final :
    public PacketImpl<RequestNetworkSettingsPacket, ProtocolInfo::requestNetworkSettingsPacket, PacketDirection::Serverbound, true>
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
} // namespace cyrex::network::mcbe::protocol
