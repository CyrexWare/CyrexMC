#include "network_session.hpp"

#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/snappy_compressor.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <mutex>
#include <sstream>
#include <vector>

using namespace cyrex::network::io;

namespace
{
std::string hexDump(const uint8_t* data, size_t len)
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
} // namespace

namespace cyrex::network::session
{

// eh, we could just call flush directly, but we might expand this function
void NetworkSession::tick()
{
    flush();
}

void NetworkSession::onRaw(const RakNet::Packet& /*packet*/, const uint8_t* data, size_t len)
{
    BinaryReader in(data, len);
    do
    {
        const std::uint32_t packetLength = in.readVarUInt();

        BinaryReader packetBuffer(data + in.offset, len);
        in.offset += packetLength;
        const std::uint32_t packetHeader = packetBuffer.readVarUInt();
        const std::uint32_t packetId = packetHeader & 0x3FF;
        cyrex::logging::info(LOG_MCBE, "packet length = {}", packetLength);
        cyrex::logging::info(LOG_MCBE, "packet id = {}0x{:02X}", cyrex::logging::Color::GOLD, packetId);

        const auto* packetDef = m_packetFactory.find(packetId);
        if (!packetDef)
        {
            cyrex::logging::error(LOG_MCBE, "unknown packet id");
            return;
        }

        const auto packet = packetDef->decode(packetBuffer);
        if (!packet)
        {
            cyrex::logging::error(LOG_MCBE, "error decoding packet");
            return;
        }
        packet->subClientId = packetHeader >> 10 & 0x03;
        if (!packet->handle(*this))
        {
            cyrex::logging::error(LOG_MCBE, "error handling packet");
            return;
        }
    } while (in.remaining() > 0);
}

bool NetworkSession::disconnectUserForIncompatibleProtocol(const uint32_t protocolVersion)
{
    auto packet = std::make_unique<mcbe::protocol::PlayStatusPacket>();
    packet->status = protocolVersion < mcbe::protocol::ProtocolInfo::currentProtocol
                        ? mcbe::protocol::PlayStatusPacket::loginFailedClient
                        : mcbe::protocol::PlayStatusPacket::loginFailedServer;

    send(std::move(packet), true);
    return true;
}

void NetworkSession::send(std::unique_ptr<mcbe::Packet> packet, const bool immediately)
{
    if (immediately)
    {
        BinaryWriter packetBuffer;

        packet->encode(packetBuffer);

        sendInternal(packetBuffer);
        return;
    }
    m_sendQueue.push_back(std::move(packet));
}

void NetworkSession::sendBatch(std::vector<std::unique_ptr<mcbe::Packet>> packets, const bool immediately)
{
    if (immediately)
    {
        BinaryWriter packetBuffer;

        for (const auto& packet : packets)
        {
            packet->encode(packetBuffer);
        }

        sendInternal(packetBuffer);
        return;
    }

    m_sendQueue.reserve(m_sendQueue.size() + packets.size());

    for (auto& packet : packets)
    {
        m_sendQueue.push_back(std::move(packet));
    }
}

void NetworkSession::flush()
{
    if (m_sendQueue.empty())
        return;

    BinaryWriter packetBuffer;

    for (const auto& packet : m_sendQueue)
    {
        packet->encode(packetBuffer);
    }

    sendInternal(packetBuffer);
    m_sendQueue.clear();
}

void NetworkSession::sendInternal(const BinaryWriter& payload) const
{
    const std::string buffer = hexDump(payload.data(), payload.length());
    logging::info("raw packet payload = {}", buffer);
    std::vector<uint8_t> out;

    if (!compressionEnabled) {
        out.assign(payload.data(), payload.data() + payload.length());
    }
    else if (compressor().networkId() == mcpe::protocol::types::CompressionAlgorithm::ZLIB ||
             compressor().networkId() == mcpe::protocol::types::CompressionAlgorithm::SNAPPY)
    {
        auto threshold = compressor().compressionThreshold().value_or(0);

        if (payload.length() >= threshold)
        {
            std::vector<uint8_t> compressed;
            switch (compressor().compress(payload.data(), payload.length(), compressed))
            {
                case mcbe::compression::CompressionStatus::FAILED:
                    cyrex::logging::error("compression failed");
                    return;
                case mcbe::compression::CompressionStatus::SUCCESS:
                    out.push_back(std::to_underlying(compressor().networkId()));
                    cyrex::logging::info("compression success");
                    break;
                case mcbe::compression::CompressionStatus::RAW:
                    out.push_back(std::to_underlying(mcpe::protocol::types::CompressionAlgorithm::NONE));
                    cyrex::logging::info("compression raw");
                    break;
            }
            out.insert(out.end(), compressed.begin(), compressed.end());
        }
        else
        {
            out.push_back(std::to_underlying(mcpe::protocol::types::CompressionAlgorithm::NONE));
            out.insert(out.end(), payload.data(), payload.data() + payload.length());
        }
    }
    else
    {
        out.push_back(std::to_underlying(mcpe::protocol::types::CompressionAlgorithm::NONE));
        out.insert(out.end(), payload.data(), payload.data() + payload.length());
    }
    if (encryptionEnabled)
    {
        const std::vector<uint8_t> old(out);
        if (mcbe::encryption::encrypt(cipherBlock(), old.data(), old.size(), out))
        {
            logging::error("encryption failed");
            return;
        }
    }
    out.insert(out.begin(), 0xFE);

    const std::string dump = hexDump(out.data(), out.size());

    logging::info("send payload = {}", dump);

    m_transport->send(m_guid, out.data(), out.size());
}

void NetworkSession::setCompressor(std::unique_ptr<mcbe::compression::Compressor> comp)
{
    m_compressor = std::move(comp);
}

bool NetworkSession::handleLogin(uint32_t version, std::string authInfoJson, std::string clientDataJwt)
{
    if (!mcbe::protocol::isProtocolMabyeAccepted(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }
    auto authData = nlohmann::json::parse(authInfoJson);
    return true;
}

bool NetworkSession::handleRequestNetworkSettings(uint32_t version)
{
    if (!mcbe::protocol::isProtocolMabyeAccepted(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }
    // this packet needs to be properly handled and we should call session's compressor networkId, right now this is just hardcoded
    auto packet = std::make_unique<mcbe::protocol::NetworkSettingsPacket>();
    packet->compressionThreshold = mcbe::protocol::NetworkSettingsPacket::compressEverything;
    packet->compressionAlgorithm = std::to_underlying(mcpe::protocol::types::CompressionAlgorithm::SNAPPY);
    packet->enableClientThrottling = false;
    packet->clientThrottleThreshold = 0;
    packet->clientThrottleScalar = 0.0f;
    send(std::move(packet), true);

    // setCompressor(std::make_unique<compression::ZlibCompressor>(6, 256, 2 * 1024 * 1024));
    setCompressor(std::make_unique<mcbe::compression::SnappyCompressor>(std::optional<size_t>{256}, 2 * 1024 * 1024));

    compressionEnabled = true;
    return true;
}

} // namespace cyrex::network::session
