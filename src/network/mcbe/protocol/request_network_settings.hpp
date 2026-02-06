#pragma once

#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/session/network_session.hpp"

#include <RakNet/RakNetTypes.h>
#include <iostream>

#include <cstdint>

namespace cyrex::nw::protocol
{

class RequestNetworkSettingsPacket final :
    public cyrex::nw::protocol::PacketImpl<RequestNetworkSettingsPacket,
                                            ProtocolInfo::requestNetworkSettingsPacket,
                                            cyrex::nw::protocol::PacketDirection::Serverbound,
                                            true>
{
public:
    uint32_t protocolVersion = 0;

protected:
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

public:
    bool handle(cyrex::nw::session::NetworkSession& session) override
    {
        return session.handleRequestNetworkSettings(protocolVersion);
    }
};
} // namespace cyrex::nw::protocol
