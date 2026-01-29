#pragma once

#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/session/network_session.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"

#include <RakNet/RakNetTypes.h>
#include <iostream>

#include <cstdint>

namespace cyrex::network::mcbe::protocol
{

class RequestNetworkSettingsPacket final : public cyrex::network::mcbe::Packet
{
public:
    using Packet::Packet;

    uint32_t protocolVersion = 0;

protected:
    bool decodePayload(cyrex::network::io::BinaryReader& in) override
    {
        protocolVersion = in.readU32BE();
        cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                       cyrex::text::format::Builder()
                                           .color(text::format::Color::DARK_GRAY)
                                           .text("[MCBE] Received Protocol Version (RequestNetworkSettingsPacket): " +
                                                 protocolVersion)
                                           .build());
        return true;
    }

    bool encodePayload(cyrex::network::io::BinaryWriter&) const override
    {
        // NOOP
        return false;
    }

public:
    bool handle(cyrex::network::session::NetworkSession& session) override
    {
        return session.handleRequestNetworkSettings(protocolVersion);
    }
};

class RequestNetworkSettingsPacketDef final : public cyrex::network::mcbe::PacketDefImpl<RequestNetworkSettingsPacket>
{
public:
    RequestNetworkSettingsPacketDef() :
        PacketDefImpl{ProtocolInfo::requestNetworkSettingsPacket, cyrex::network::mcbe::PacketDirection::Serverbound, true}
    {
    }
};
} // namespace cyrex::network::mcbe::protocol
