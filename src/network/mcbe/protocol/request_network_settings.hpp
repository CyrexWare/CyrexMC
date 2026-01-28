#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/session/network_session.hpp"
#include "util/textformat.hpp"

#include <RakNet/RakNetTypes.h>
#include <iostream>

#include <cstdint>

namespace cyrex::network::mcbe::protocol
{

class RequestNetworkSettingsPacket final : public cyrex::network::mcbe::Packet
{
public:
    uint32_t protocolVersion = 0;

    [[nodiscard]] [[nodiscard]] uint32_t networkId() const override
    {
        return ProtocolInfo::requestNetworkSettingsPacket;
    }

    [[nodiscard]] [[nodiscard]] cyrex::network::mcbe::PacketDirection direction() const override
    {
        return cyrex::network::mcbe::PacketDirection::Serverbound;
    }

    [[nodiscard]] [[nodiscard]] bool allowBeforeLogin() const override
    {
        return true;
    }

protected:
    void decodePayload(cyrex::network::io::BinaryReader& in) override
    {
        protocolVersion = in.readU32BE();
        std::cout << (cyrex::util::renderConsole(cyrex::util::bedrock(cyrex::util::Color::GREEN) + "[MCBE] ", true) +
                      cyrex::util::renderConsole(cyrex::util::bedrock(cyrex::util::Color::EMERALD) +
                                                     "Received "
                                                     "Protocol Version "
                                                     "(RequestNetworkSe"
                                                     "ttings): ",
                                                 false))
                  << protocolVersion << std::endl;
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
