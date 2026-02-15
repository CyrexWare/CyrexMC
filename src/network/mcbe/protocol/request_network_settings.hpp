#pragma once

#include "log/logging.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/session/network_session.hpp"

#include <RakNet/RakNetTypes.h>
#include <iostream>

#include <cstdint>

namespace cyrex::nw::protocol
{

class RequestNetworkSettingsPacket final :
    public cyrex::nw::protocol::PacketImpl<RequestNetworkSettingsPacket,
                                           static_cast<uint32_t>(PacketId::RequestNetworkSettings),
                                           cyrex::nw::protocol::PacketDirection::Serverbound,
                                           true>
{
public:
    uint32_t protocolVersion = 0;

    bool decodePayload(cyrex::nw::io::BinaryReader& in) override
    {
        protocolVersion = in.readU32BE();
        cyrex::logging::info(LOG_MCBE, "Received Protocol Version (RequestNetworkSettingsPacket): {}", protocolVersion);
        return true;
    }

    bool encodePayload(cyrex::nw::io::BinaryWriter&) const override
    {
        // NOOP
        return false;
    }

    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return session.handleRequestNetworkSettings(protocolVersion);
    }
};
} // namespace cyrex::nw::protocol
