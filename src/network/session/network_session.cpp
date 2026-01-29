#include "network_session.hpp"

#include "log/console_logger.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/handler/session_begin_handler.hpp"
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
                                .text(std::to_string(packetId))
                                .build());

    auto mcbePacket = cyrex::network::mcbe::PacketPool::instance().create(packetId);
    if (!mcbePacket)
    {
        log::sendConsoleMessage(log::MessageType::MCBE_ERROR,
                                text::format::Builder().color(text::format::Color::DARK_GRAY).text("unknown packet id").build());
        return;
    }

    mcbePacket->decode(in);
    mcbePacket->handle(*this);
}

bool NetworkSession::disconnectUserForIncompatiableProtocol(uint32_t protocolVersion)
{
    cyrex::network::mcbe::protocol::PlayStatusPacket packet{};
    packet.status = protocolVersion < cyrex::network::mcbe::protocol::ProtocolInfo::currentProtocol
                        ? cyrex::network::mcbe::protocol::PlayStatusPacket::loginFailedClient
                        : cyrex::network::mcbe::protocol::PlayStatusPacket::loginFailedServer;

    send(packet);
    return true;
}

void NetworkSession::send(cyrex::network::mcbe::PacketBase& packet, bool immediately)
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

void NetworkSession::sendInternal(cyrex::network::mcbe::PacketBase& packet)
{
    if (cyrex::network::mcbe::PacketPool::instance().direction(packet.networkId()) ==
        cyrex::network::mcbe::PacketDirection::Serverbound)
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
                log::sendConsoleMessage(log::MessageType::E_RROR,
                                        text::format::Builder()
                                            .color(text::format::Color::DARK_GRAY)
                                            .text("compression failed")
                                            .build());
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

    log::sendConsoleMessage(log::MessageType::DEBUG,
                            text::format::Builder()
                                .color(text::format::Color::DARK_GRAY)
                                .text("send packet id=")
                                .color(text::format::Color::GOLD)
                                .text(std::to_string(packet.networkId()))
                                .build());

    const std::string dump = hexDump(out.data(), out.size());

    log::sendConsoleMessage(log::MessageType::DEBUG,
                            text::format::Builder()
                                .color(text::format::Color::DARK_GRAY)
                                .text("send payload: ")
                                .color(text::format::Color::GRAY)
                                .text(dump)
                                .build());

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
