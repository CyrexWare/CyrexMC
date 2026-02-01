#include "network_session.hpp"

#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"

#include <iomanip>
#include <iostream>
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

    const uint32_t packetLength = in.readVarUInt();
    const uint32_t packetId = in.readVarUInt();

    cyrex::logging::info(LOG_MCBE, "packet length = {}", packetLength);
    cyrex::logging::info(LOG_MCBE, "packet id = {}0x{:02X}", cyrex::logging::Color::GOLD, packetId);

    const auto* packetDef = m_packetFactory.find(packetId);
    if (!packetDef)
    {
        cyrex::logging::error(LOG_MCBE, "unknown packet id");
        return;
    }

    auto packet = packetDef->decode(in);
    if (!packet)
    {
        cyrex::logging::error(LOG_MCBE, "error decoding packet");
        return;
    }

    if (!packet->handle(*this))
    {
        cyrex::logging::error(LOG_MCBE, "error handling packet");
        return;
    }
}

bool NetworkSession::disconnectUserForIncompatiableProtocol(uint32_t protocolVersion)
{
    cyrex::network::mcbe::protocol::PlayStatusPacket packet;
    packet.status = protocolVersion < cyrex::network::mcbe::protocol::ProtocolInfo::currentProtocol
                         ? cyrex::network::mcbe::protocol::PlayStatusPacket::loginFailedClient
                         : cyrex::network::mcbe::protocol::PlayStatusPacket::loginFailedServer;

    send(packet);
    return true;
}

void NetworkSession::send(cyrex::network::mcbe::Packet& packet, bool immediately)
{
    if (immediately)
    {
        sendInternal(packet);
        return;
    }

    m_sendQueue.emplace([this, &packet]() { sendInternal(packet); });
}

void NetworkSession::flush()
{
    while (!m_sendQueue.empty())
    {
        auto& fn = m_sendQueue.front();
        fn();
        m_sendQueue.pop();
    }
}

void NetworkSession::sendInternal(cyrex::network::mcbe::Packet& packet)
{
    if (packet.getDef().direction == cyrex::network::mcbe::PacketDirection::Serverbound)
    {
        return;
    }

    BinaryWriter payload;
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
                cyrex::logging::error(LOG_MCBE, "compression failed");
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

    logging::info(LOG_MCBE, "send packet id = {}0x{:02X}", logging::Color::GOLD, packet.getDef().networkId);

    const std::string dump = hexDump(out.data(), out.size());

    logging::info(LOG_MCBE, "send payload = {}", dump);

    m_transport->send(m_guid, out.data(), out.size());
}

void NetworkSession::setCompressor(std::unique_ptr<cyrex::network::mcbe::compression::Compressor> comp)
{
    m_compressor = std::move(comp);
}

bool NetworkSession::handleRequestNetworkSettings(uint32_t version)
{
    // no guarentee that a protocol is accepted at the moment
    // we mabye will support multiple versions or make it easy for plugins to.
    if (!cyrex::network::mcbe::protocol::isProtocolMabyeAccepted(version))
    {
        //disconnectUserForIncompatiableProtocol(version);
        return false;
    }

    setProtocolId(version);

    // this packet needs to be properly handled and we should call session's compressor networkId, right now this is just hardcoded
    cyrex::network::mcbe::protocol::NetworkSettingsPacket packet;;
    packet.compressionThreshold = cyrex::network::mcbe::protocol::NetworkSettingsPacket::compressEverything;
    packet.compressionAlgorithm = 0;
    packet.enableClientThrottling = false;
    packet.clientThrottleThreshold = 0;
    packet.clientThrottleScalar = 0.0f;
    send(packet, true);
    // mark compression as ready to go lol!
    compressionEnabled = true;

    return true;
}

} // namespace cyrex::network::session
