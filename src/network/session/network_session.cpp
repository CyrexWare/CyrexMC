#include "network_session.hpp"

#include "network/mcbe/handler/session_begin_handler.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/util/binary_stream.hpp"
#include "util/textformat.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
using namespace cyrex::util;
using namespace cyrex::network::util;

namespace cyrex::network::session
{

void NetworkSession::onRaw(RakNet::Packet, const uint8_t* data, size_t len)
{
    BinaryStream in(data, len);

    uint32_t packetLength = in.readVarUInt();
    uint32_t packetId = in.readVarUInt();

    std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
              << renderConsole(bedrock(Color::DARK_GRAY) + " packet length = ", false) << packetLength << std::endl;

    std::cout << renderConsole(bedrock(Color::GREEN) + "[MCBE][DEBUG]", true)
              << renderConsole(bedrock(Color::DARK_GRAY) + " packet id = 0x", false) << std::hex << packetId << std::dec
              << std::endl;

    auto packet = cyrex::network::mcbe::PacketPool::instance().create(packetId);
    if (!packet)
    {
        std::cout << renderConsole(bedrock(Color::RED) + "[MCBE][ERROR]", true)
                  << renderConsole(bedrock(Color::DARK_GRAY) + " unknown packet id", false) << std::endl;
        return;
    }

    packet->decode(in);
    packet->handle(*this);
}

static std::string hexDump(const uint8_t* data, size_t len)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0; i < len; ++i)
    {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if (i + 1 < len)
            oss << ' ';
    }

    return oss.str();
}

// TODO: give better name
bool NetworkSession::disconnectUserForIncompatiableProtocol(uint32_t protocolVersion)
{
    cyrex::network::mcbe::protocol::PlayStatusPacket packet{};
    packet.status = protocolVersion < cyrex::network::mcbe::protocol::ProtocolInfo::CURRENT_PROTOCOL
                        ? cyrex::network::mcbe::protocol::PlayStatusPacket::LOGIN_FAILED_CLIENT
                        : cyrex::network::mcbe::protocol::PlayStatusPacket::LOGIN_FAILED_SERVER;
    send(packet);
    return true;
}

void NetworkSession::send(cyrex::network::mcbe::PacketBase& packet)
{
    if (cyrex::network::mcbe::PacketPool::instance().direction(packet.networkId()) ==
        cyrex::network::mcbe::PacketDirection::Serverbound)
    {
        return;
    }

    cyrex::network::util::BinaryStream payload;
    packet.encode(payload);

    std::vector<uint8_t> out;

    if (!compressionEnabled || compressor().networkId() == mcpe::protocol::types::CompressionAlgorithm::NONE)
    {
        out.assign(payload.data(), payload.data() + payload.length());
    }
    else if (compressor().networkId() == mcpe::protocol::types::CompressionAlgorithm::ZLIB)
    {
        auto threshold = compressor().compressionThreshold().value_or(0);

        if (payload.length() >= threshold)
        {
            std::vector<uint8_t> compressed;

            if (!compressor().compress(payload.data(), payload.length(), compressed))
            {
                std::cout << renderConsole(bedrock(Color::RED) + "[Cyrex][ERROR]", true)
                          << renderConsole(bedrock(Color::DARK_GRAY) + " compression failed", false) << std::endl;
                return;
            }

            out.push_back(static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::ZLIB));
            out.insert(out.end(), compressed.begin(), compressed.end());
        }
        else
        {
            out.push_back(static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::NONE));
            out.insert(out.end(), payload.data(), payload.data() + payload.length());
        }
    }

    std::string msg = bedrock(Color::BLUE) + "[Cyrex] " + bedrock(Color::DARK_GRAY) +
                      "send packet id=" + bedrock(Color::GOLD) + std::to_string(packet.networkId());

    std::cout << renderConsole(msg, true) << std::endl;

    std::string dump = hexDump(out.data(), out.size());

    std::cout << renderConsole(bedrock(Color::BLUE) + "[Cyrex]", true)
              << renderConsole(bedrock(Color::DARK_GRAY) + " send payload: " + bedrock(Color::GRAY) + dump, false)
              << std::endl;

    m_transport->send(m_guid, out.data(), out.size());
}

void NetworkSession::setCompressor(std::unique_ptr<cyrex::network::mcbe::compression::Compressor> comp)
{
    m_compressor = std::move(comp);
}

bool NetworkSession::handleRequestNetworkSettings(cyrex::network::mcbe::PacketBase& packet)
{
    cyrex::network::mcbe::handler::SessionBeginHandler handler{};
    handler.handle(*this, packet);
    return true;
}
} // namespace cyrex::network::session