#pragma once

#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_direction.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/session/network_session.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "util/textformat.hpp"

#include <RakNet/RakNetTypes.h>
#include <iostream>

#include <cstdint>

// is this valid?
using namespace cyrex::network::io;
using namespace cyrex::network::session;

namespace cyrex::network::mcbe::protocol
{

class RequestNetworkSettingsPacket final : public cyrex::network::mcbe::Packet
{
public:
    uint32_t protocolVersion = 0;

    uint32_t networkId() const override
    {
        return ProtocolInfo::REQUEST_NETWORK_SETTINGS_PACKET;
    }

    cyrex::network::mcbe::PacketDirection direction() const override
    {
        return cyrex::network::mcbe::PacketDirection::Serverbound;
    }

    bool allowBeforeLogin() const override
    {
        return true;
    }

protected:
    void decodePayload(BinaryReader& in) override
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

    void encodePayload(BinaryWriter&) const override
    {
        // NOOP
    }

public:
    bool handle(NetworkSession& session) override
    {
        session.handleRequestNetworkSettings(*this);
        return true;
    }
};
} // namespace cyrex::network::mcbe::protocol