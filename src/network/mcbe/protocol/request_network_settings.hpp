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
    uint32_t protocolVersion = 0;

    [[nodiscard]] uint32_t networkId() const override
    {
        return ProtocolInfo::requestNetworkSettingsPacket;
    }

    [[nodiscard]] cyrex::network::mcbe::PacketDirection direction() const override
    {
        return cyrex::network::mcbe::PacketDirection::Serverbound;
    }

    [[nodiscard]] bool allowBeforeLogin() const override
    {
        return true;
    }

protected:
    void decodePayload(cyrex::network::io::BinaryReader& in) override
    {
        protocolVersion = in.readU32BE();
        cyrex::log::sendConsoleMessage(cyrex::log::MessageType::MCBE_DEBUG,
                                       cyrex::text::format::Builder()
                                           .color(text::format::Color::DARK_GRAY)
                                           .text("[MCBE] Received Protocol Version (RequestNetworkSettingsPacket): " +
                                                 protocolVersion)
                                           .build());
    }

    void encodePayload(cyrex::network::io::BinaryWriter&) const override
    {
        // NOOP
    }

public:
    bool handle(cyrex::network::session::NetworkSession& session) override
    {
        session.handleRequestNetworkSettings(*this);
        return true;
    }
};
} // namespace cyrex::network::mcbe::protocol
