#include "network_session.hpp"

#include "log/console_logger.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "text/format/builder.hpp"

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

    log::sendConsoleMessage(log::MessageType::MCBE_DEBUG,
                            text::format::Builder()
                                .color(text::format::Color::DARK_GRAY)
                                .text("packet length = ")
                                .color(text::format::Color::GOLD)
                                .text(std::to_string(packetLength))
                                .build());

    log::sendConsoleMessage(log::MessageType::MCBE_DEBUG,
                            text::format::Builder()
                                .color(text::format::Color::DARK_GRAY)
                                .text("packet id = 0x")
                                .color(text::format::Color::GOLD)
                                .text(std::format("{:02X}", static_cast<uint32_t>(packetId)))
                                .build());

    const auto* packetDef = m_packetFactory.find(packetId);
    if (!packetDef)
    {
        log::sendConsoleMessage(log::MessageType::MCBE_ERROR,
                                text::format::Builder().color(text::format::Color::DARK_GRAY).text("unknown packet id").build());
        return;
    }

    auto packet = packetDef->decode(in);
    if (!packet)
    {
        log::sendConsoleMessage(log::MessageType::MCBE_ERROR,
                                text::format::Builder()
                                    .color(text::format::Color::DARK_GRAY)
                                    .text("error decoding packet")
                                    .build());
        return;
    }

    if (!packet->handle(*this))
    {
        log::sendConsoleMessage(log::MessageType::MCBE_ERROR,
                                text::format::Builder()
                                    .color(text::format::Color::DARK_GRAY)
                                    .text("error handling packet")
                                    .build());
        return;
    }
}

bool NetworkSession::disconnectUserForIncompatiableProtocol(uint32_t protocolVersion)
{
    auto packet = m_packetFactory.create<cyrex::network::mcbe::protocol::PlayStatusPacketDef>();
    packet->status = protocolVersion < cyrex::network::mcbe::protocol::ProtocolInfo::currentProtocol
                         ? cyrex::network::mcbe::protocol::PlayStatusPacket::loginFailedClient
                         : cyrex::network::mcbe::protocol::PlayStatusPacket::loginFailedServer;

    send(*packet);
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
    else if (
        compressor().networkId() == mcpe::protocol::types::CompressionAlgorithm::ZLIB
        || compressor().networkId() == mcpe::protocol::types::CompressionAlgorithm::SNAPPY
        )
    {
        auto threshold = compressor().compressionThreshold().value_or(0);

        if (payload.length() >= threshold)
        {
            std::vector<uint8_t> compressed;
            switch (compressor().compress(payload.data(), payload.length(), compressed)) {
                case mcbe::compression::CompressionStatus::FAILED:
                    log::sendConsoleMessage(log::MessageType::E_RROR,
                                        text::format::Builder()
                                            .color(text::format::Color::DARK_GRAY)
                                            .text("compression failed")
                                            .build());
                    return;
                case mcbe::compression::CompressionStatus::SUCCESS:
                    out.push_back(static_cast<uint8_t>(compressor().networkId()));
                    break;
                case mcbe::compression::CompressionStatus::RAW:
                    out.push_back(static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::NONE));
                    break;
            }
            out.insert(out.end(), compressed.begin(), compressed.end());
        }
        else
        {
            out.push_back(static_cast<uint8_t>(mcpe::protocol::types::CompressionAlgorithm::NONE));
            out.insert(out.end(), payload.data(), payload.data() + payload.length());
        }
    }
    if (encryptionEnabled)
    {
        out.clear();//TODO: encryption
    }
    out.insert(out.begin(), 0xFE);
    const std::string dump = hexDump(out.data(), out.size());

    log::sendConsoleMessage(log::MessageType::DEBUG,
                            text::format::Builder()
                                .color(text::format::Color::DARK_GRAY)
                                .text("send payload = ")
                                .color(text::format::Color::GRAY)
                                .text(dump)
                                .build());

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
    auto packet = m_packetFactory.create<cyrex::network::mcbe::protocol::NetworkSettingsPacketDef>();
    packet->compressionThreshold = cyrex::network::mcbe::protocol::NetworkSettingsPacket::compressEverything;
    packet->compressionAlgorithm = 1;
    packet->padding = 00;
    packet->enableClientThrottling = false;
    packet->clientThrottleThreshold = 0;
    packet->clientThrottleScalar = 0.0f;
    packet->trailingZero = 0;
    send(*packet, true);
    // mark compression as ready to go lol!
    compressionEnabled = true;

    return true;
}

} // namespace cyrex::network::session
