#include "network_session.hpp"

#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/compressors.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_request.hpp"
#include "network/mcbe/protocol/resource_pack_client_response.hpp"
#include "network/mcbe/protocol/resource_pack_data_info.hpp"
#include "network/mcbe/protocol/resource_pack_stack.hpp"
#include "network/mcbe/protocol/resource_packs_info.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackEntry.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "network/raknet/handler/raknet_handler.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include <cstdint>

using namespace cyrex::nw::io;

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

cyrex::nw::io::UUID randomUUID()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());

    std::uniform_int_distribution<unsigned int> dist(0, 255);

    cyrex::nw::io::UUID uuid{};

    for (auto& b : uuid)
        b = static_cast<uint8_t>(dist(gen));

    uuid[6] = (uuid[6] & 0x0F) | 0x40;
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    return uuid;
}

} // namespace

namespace cyrex::nw::session
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
        cyrex::logging::info(LOG_MCBE,
                             "packet id = {}0x{:02X} ({})",
                             cyrex::logging::Color::GOLD,
                             packetId,
                             cyrex::nw::protocol::toSimpleName(cyrex::nw::protocol::fromInt(packetId)));

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
    auto packet = std::make_unique<protocol::PlayStatusPacket>();
    packet->status = protocolVersion < protocol::ProtocolInfo::currentProtocol
                         ? protocol::PlayStatusPacket::loginFailedClient
                         : protocol::PlayStatusPacket::loginFailedServer;

    send(std::move(packet), true);
    return true;
}

void NetworkSession::send(std::unique_ptr<protocol::Packet> packet, const bool immediately)
{
    cyrex::logging::info("queueing packet with id = {}0x{:02X} ({})",
                         cyrex::logging::Color::GOLD,
                         packet->getDef().networkId,
                         cyrex::nw::protocol::toSimpleName(cyrex::nw::protocol::fromInt(packet->getDef().networkId)));
    if (immediately)
    {
        BinaryWriter packetBuffer;

        packet->encode(packetBuffer);

        sendInternal(packetBuffer);
        return;
    }
    m_sendQueue.push_back(std::move(packet));
}

void NetworkSession::sendBatch(std::vector<std::unique_ptr<protocol::Packet>> packets, const bool immediately)
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

void NetworkSession::sendInternal(const BinaryWriter& payload)
{
    const std::string buffer = hexDump(payload.data(), payload.length());
    logging::info("raw packet payload = {}", buffer);
    std::vector<uint8_t> out;

    if (!compressionEnabled)
    {
        out = payload.buffer;
    }
    else
    {
        const auto* comp = protocol::getCompressor(compressor);
        if (comp && comp->shouldCompress(payload.length()))
        {
            std::vector<uint8_t> compressed = *comp->compress(payload.buffer);

            out.push_back(std::to_underlying(compressor));
            logging::info("compression success");

            out.insert(out.end(), compressed.begin(), compressed.end());
        }
        else
        {
            out.push_back(std::to_underlying(protocol::CompressionAlgorithm::NONE));
            out.insert(out.end(), payload.data(), payload.data() + payload.length());
        }
    }

    if (encryptionEnabled)
    {
        const std::vector old(out);
        if (auto data = getEncryptor().encrypt(old))
        {
            out = std::move(*data);
        }
        else
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

bool NetworkSession::handleLogin(const uint32_t version, const std::string& authInfoJson, const std::string& clientDataJwt)
{
    if (!protocol::isSupportedProtocol(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }
    auto authData = nlohmann::json::parse(authInfoJson);

    doLoginSuccess();
    return true;
}

void NetworkSession::doLoginSuccess()
{
    auto packet = std::make_unique<protocol::PlayStatusPacket>();
    packet->status = protocol::PlayStatusPacket::loginSuccess;
    send(std::move(packet));
    // Encryption below (for ency)

    // Initial resource pack setup
    auto infoPkt = std::make_unique<protocol::ResourcePacksInfoPacket>();
    infoPkt->resourcePackEntries = m_server.getResourcePackFactory().getResourceStack();
    infoPkt->mustAccept = m_server.shouldForceResources();
    // future making this configurable
    infoPkt->disableVibrantVisuals = false;
    infoPkt->worldTemplateId = randomUUID();
    infoPkt->worldTemplateVersion = "";
    send(std::move(infoPkt));
}

bool NetworkSession::handleRequestNetworkSettings(const uint32_t version)
{
    if (!protocol::isSupportedProtocol(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }

    compressor = protocol::CompressionAlgorithm::ZLIB;

    // this packet needs to be properly handled, and we should call session's compressor networkId, right now this is just hardcoded
    auto packet = std::make_unique<protocol::NetworkSettingsPacket>();
    packet->compressionThreshold = protocol::NetworkSettingsPacket::compressEverything;
    packet->compressionAlgorithm = std::to_underlying(compressor);
    packet->enableClientThrottling = false;
    packet->clientThrottleThreshold = 0;
    packet->clientThrottleScalar = 0.0f;
    send(std::move(packet), true);

    compressionEnabled = true;

    return true;
}

bool NetworkSession::handleResourcePackClientResponse(const cyrex::nw::protocol::ResourcePackClientResponsePacket& pk)
{
    using cyrex::nw::protocol::ResourcePackClientResponseStatus;

    switch (pk.responseStatus)
    {
        case ResourcePackClientResponseStatus::Refused:
            logging::warn("client refused resource packs");
            if (m_server.shouldForceResources())
            {
                logging::info("disconnecting client for refusing required resource packs");
                markedForDisconnect = true;
            }
            break;

        case ResourcePackClientResponseStatus::SendPacks:
            logging::info("client is downloading resource packs");
            for (const auto& entry : pk.packEntries)
            {
                auto resourcePack = m_server.getResourcePackFactory().getPackById(entry.uuid);
                if (!resourcePack)
                {
                    logging::warn("client requested unknown resource pack, disconnecting");
                    markedForDisconnect = true;
                    return true;
                }

                int maxChunkSize = m_server.getResourcePackFactory().getMaxChunkSize();
                int chunkCount = static_cast<int>(
                    std::ceil(static_cast<double>(resourcePack->getPackSize()) / maxChunkSize));

                // packs[resourcePack->getPackId()] = std::make_shared<PackMeta>(resourcePack->getPackId(),
                //                                                               resourcePack,
                //                                                               maxChunkSize,
                //                                                               chunkCount);

                auto dataInfoPkt = std::make_unique<protocol::ResourcePackDataInfoPacket>();
                dataInfoPkt->packId = resourcePack->getPackId();
                dataInfoPkt->packVersion = resourcePack->getPackVersion();
                dataInfoPkt->maxChunkSize = maxChunkSize;
                dataInfoPkt->chunkCount = chunkCount;
                dataInfoPkt->compressedPackSize = resourcePack->getPackSize();
                dataInfoPkt->sha256 = resourcePack->getSha256();
                send(std::move(dataInfoPkt));
            }
            break;

        case ResourcePackClientResponseStatus::HaveAllPacks:
            logging::info("client has all required resource packs");

            {
                auto stackPkt = std::make_unique<protocol::ResourcePackStackPacket>();
                stackPkt->mustAccept = m_server.shouldForceResources();
                auto packs = m_server.getResourcePackFactory().getResourceStack();
                stackPkt->resourcePackStack.clear();

                for (auto& packDefPtr : packs)
                {
                    if (packDefPtr)
                        stackPkt->resourcePackStack.emplace_back(*packDefPtr);
                }


                send(std::move(stackPkt));
            }
            break;

        case ResourcePackClientResponseStatus::Completed:
            logging::info("client has completed resource pack processing");
            break;
    }

    return true;
}

bool NetworkSession::handleResourcePackChunkRequest(const cyrex::nw::protocol::ResourcePackChunkRequestPacket& pk)
{
    return true;
}

} // namespace cyrex::nw::session
