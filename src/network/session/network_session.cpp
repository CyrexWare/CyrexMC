#include "network_session.hpp"

#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/compressors.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/network_settings.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_data.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_request.hpp"
#include "network/mcbe/protocol/resource_pack_client_response.hpp"
#include "network/mcbe/protocol/resource_pack_data_info.hpp"
#include "network/mcbe/protocol/resource_pack_stack.hpp"
#include "network/mcbe/protocol/resource_packs_info.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackInfoEntry.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackMeta.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackStackEntry.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "network/raknet/handler/raknet_handler.hpp"
#include "util/uuid.hpp"

#include <algorithm>
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

#ifdef min
#undef min
#endif

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
} // namespace

namespace cyrex::nw::session
{

void NetworkSession::tick()
{
    flush();
}

void NetworkSession::onRaw(const Packet& /*packet*/, const uint8_t* data, const size_t len)
{
    BinaryReader in(data, len);
    do
    {
        const std::uint32_t packetLength = in.readVarUInt();

        BinaryReader packetBuffer(data + in.offset, len);
        in.offset += packetLength;
        const std::uint32_t packetHeader = packetBuffer.readVarUInt();
        const std::uint32_t packetId = packetHeader & 0x3FF;
        logging::info(LOG_MCBE, "packet length = {}", packetLength);
        logging::info(LOG_MCBE,
                      "packet id = {}0x{:02X} ({})",
                      logging::Color::GOLD,
                      packetId,
                      proto::toReadablePacketName(nw::proto::makePacketId(packetId)));

        if (packetId != 0x01)
        {
            std::stringstream ss;
            const auto* payload = data + in.offset - packetLength + packetBuffer.offset;
            const size_t payloadSize = packetLength - (packetBuffer.offset);
            ss << "raw payload (" << payloadSize << " bytes): ";
            for (size_t i = 0; i < payloadSize; ++i)
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(payload[i]) << " ";
            logging::info(LOG_MCBE, "{}", ss.str());
        }

        const auto* packetDef = m_packetFactory.find(packetId);
        if (!packetDef)
        {
            logging::error(LOG_MCBE, "unknown packet id");
            return;
        }

        const auto packet = packetDef->decode(packetBuffer);
        if (!packet)
        {
            logging::error(LOG_MCBE, "error decoding packet");
            return;
        }
        packet->subClientId = packetHeader >> 10 & 0x03;
        if (!packet->handle(*this))
        {
            logging::error(LOG_MCBE, "error handling packet");
            return;
        }
    } while (in.remaining() > 0);
}

bool NetworkSession::disconnectUserForIncompatibleProtocol(const uint32_t protocolVersion)
{
    auto packet = std::make_unique<proto::PlayStatusPacket>();
    packet->status = protocolVersion < proto::ProtocolInfo::currentProtocol
                         ? proto::PlayStatus::LoginFailedClient
                         : proto::PlayStatus::LoginFailedServer;

    send(std::move(packet), true);
    return true;
}

void NetworkSession::send(std::unique_ptr<proto::Packet> packet, const bool immediately)
{
    logging::info("queueing packet with id = {}0x{:02X} ({})",
                  logging::Color::GOLD,
                  packet->getDef().networkId,
                  proto::toReadablePacketName(proto::makePacketId(packet->getDef().networkId)));
    if (immediately)
    {
        BinaryWriter packetBuffer;

        packet->encode(packetBuffer);

        sendInternal(packetBuffer);
        return;
    }
    m_sendQueue.push_back(std::move(packet));
}

void NetworkSession::sendBatch(std::vector<std::unique_ptr<proto::Packet>> packets, const bool immediately)
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
    //const std::string buffer = hexDump(payload.data(), payload.length());
    //logging::info("raw packet payload = {}", buffer);
    std::vector<uint8_t> out;

    if (!compressionEnabled)
    {
        out = payload.getBuffer();
    }
    else
    {
        const auto* comp = proto::getCompressor(compressor);
        if (comp && comp->shouldCompress(payload.size()))
        {
            std::vector<uint8_t> compressed = *comp->compress(payload.getBuffer());

            out.push_back(std::to_underlying(compressor));
            logging::info("compression success");

            out.insert(out.end(), compressed.begin(), compressed.end());
        }
        else
        {
            out.push_back(std::to_underlying(proto::CompressionAlgorithm::NONE));
            out.insert(out.end(), payload.data(), payload.data() + payload.size());
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

    //const std::string dump = hexDump(out.data(), out.size());

    //logging::info("send payload = {}", dump);

    m_transport->send(m_guid, out.data(), out.size());
}

bool NetworkSession::handleLogin(const uint32_t version, const std::string& authInfoJson, const std::string& clientDataJwt)
{
    if (!proto::isSupportedProtocol(version))
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
    auto packet = std::make_unique<proto::PlayStatusPacket>();
    packet->status = proto::PlayStatus::LoginSuccess;
    send(std::move(packet));

    auto infoPkt = std::make_unique<proto::ResourcePacksInfoPacket>();
    const auto& defs = m_server.getResourcePackFactory().getResourceStack();
    infoPkt->resourcePackEntries.reserve(defs.size());

    for (const auto& defPtr : defs)
    {
        if (!defPtr)
            continue;

        const auto& def = *defPtr;

        int chunkCount = static_cast<int>(
            std::ceil(static_cast<double>(def.getPackSize()) / m_server.getResourcePackFactory().getMaxChunkSize()));

        logging::log("chunk={}", chunkCount);

        proto::ResourcePackInfoEntry entry;
        entry.packId = def.getPackId();
        entry.packVersion = def.getPackVersion();
        entry.packSize = def.getPackSize();
        entry.encryptionKey = def.getEncryptionKey();
        entry.subPackName = "";

        entry.contentIdentity = uuid::toString(entry.packId);
        entry.scripting = def.usesScript();
        entry.addonPack = def.isAddonPack();
        entry.raytracingCapable = def.isRaytracingCapable();
        entry.cdnUrl = def.cdnUrl();

        infoPkt->resourcePackEntries.emplace_back(std::move(entry));
    }

    infoPkt->mustAccept = m_server.shouldForceResources();
    infoPkt->disableVibrantVisuals = false;

    infoPkt->worldTemplateId = uuid::randomUUID();
    infoPkt->worldTemplateVersion = "";

    send(std::move(infoPkt));
}

bool NetworkSession::handleRequestNetworkSettings(const uint32_t version)
{
    if (!proto::isSupportedProtocol(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }

    compressor = proto::CompressionAlgorithm::ZLIB;

    // this packet needs to be properly handled, and we should call session's compressor networkId, right now this is just hardcoded
    auto packet = std::make_unique<proto::NetworkSettingsPacket>();
    packet->compressionThreshold = proto::NetworkSettingsPacket::compressEverything;
    packet->compressionAlgorithm = std::to_underlying(compressor);
    packet->enableClientThrottling = false;
    packet->clientThrottleThreshold = 0;
    packet->clientThrottleScalar = 0.0f;
    send(std::move(packet), true);

    compressionEnabled = true;

    return true;
}


bool NetworkSession::handleResourcePackClientResponse(const proto::ResourcePackClientResponsePacket& pk)
{
    using proto::ResourcePackClientResponseStatus;

    switch (pk.responseStatus)
    {
        case ResourcePackClientResponseStatus::Refused:
        {
            logging::warn("client refused resource packs");
            if (m_server.shouldForceResources())
                markedForDisconnect = true;
            break;
        }

        case ResourcePackClientResponseStatus::SendPacks:
        {
            for (const auto& entry : pk.packEntries)
            {
                auto resourcePack = m_server.getResourcePackFactory().getPackById(entry.uuid);
                if (!resourcePack)
                {
                    printf("%s", uuid::toString(entry.uuid).c_str());
                    markedForDisconnect = true;
                    return true;
                }

                int maxChunkSize = m_server.getResourcePackFactory().getMaxChunkSize();
                int chunkCount = static_cast<int>(
                    std::ceil(static_cast<double>(resourcePack->getPackSize()) / maxChunkSize));

                auto data = std::make_unique<proto::ResourcePackMeta>(resourcePack->getPackId(), resourcePack, maxChunkSize, chunkCount);

                packQueue.push_back(data->packId);
                loadedPacks.emplace(data->packId, std::move(data));

                auto dataInfoPkt = std::make_unique<proto::ResourcePackDataInfoPacket>();
                dataInfoPkt->packId = resourcePack->getPackId();
                dataInfoPkt->packVersion = resourcePack->getPackVersion();
                dataInfoPkt->maxChunkSize = maxChunkSize;
                dataInfoPkt->chunkCount = chunkCount;
                dataInfoPkt->compressedPackSize = resourcePack->getPackSize();
                dataInfoPkt->sha256 = resourcePack->getSha256();
                send(std::move(dataInfoPkt), true);
            }
            break;
        }

        case ResourcePackClientResponseStatus::HaveAllPacks:
        {
            auto stackPkt = std::make_unique<proto::ResourcePackStackPacket>();
            stackPkt->mustAccept = m_server.shouldForceResources();

            const auto& defStack = m_server.getResourcePackFactory().getResourceStack();
            stackPkt->resourcePackStack.reserve(defStack.size());

            for (const auto& def : defStack)
            {
                proto::ResourcePackStackEntry entry;
                entry.packId = uuid::toString(def->getPackId());
                entry.packVersion = def->getPackVersion();
                entry.subPackName = def->getSubPackName();
                stackPkt->resourcePackStack.push_back(std::move(entry));
            }

            stackPkt->baseGameVersion = proto::ProtocolInfo::minecraftVersionNetwork;
            stackPkt->experiments = {};
            send(std::move(stackPkt), true);
            break;
        }

        case ResourcePackClientResponseStatus::Completed:
            logging::log("client has completed process");
            break;
    }

    return true;
}

bool NetworkSession::handleResourcePackChunkRequest(const proto::ResourcePackChunkRequestPacket& request)
{
    proto::ResourcePackMeta* packInfoPtr = nullptr;

    auto packFinder = loadedPacks.find(request.packId);

    if (packFinder == loadedPacks.end())
    {
        const auto rawPack = m_server.getResourcePackFactory().getPackById(request.packId);
        if (!rawPack)
        {
            markedForDisconnect = true;
            return true;
        }

        int maxSize = m_server.getResourcePackFactory().getMaxChunkSize();
        int totalChunks = static_cast<int>((rawPack->getPackSize() + maxSize - 1) / maxSize);

        auto packInfo = std::make_unique<proto::ResourcePackMeta>(rawPack->getPackId(), rawPack, maxSize, totalChunks);

        packInfoPtr = loadedPacks.emplace(packInfo->packId, std::move(packInfo)).first->second.get();
    }
    else
    {
        packInfoPtr = packFinder->second.get();
    }

    auto& packInfo = *packInfoPtr;

    if (request.chunkIndex >= 0 && request.chunkIndex < packInfo.chunkCount)
    {
        packInfo.want[static_cast<size_t>(request.chunkIndex)] = true;

        pendingChunks.emplace_back(packInfo.packId, static_cast<int>(request.chunkIndex));
    }

    if (currentPack.is_nil())
    {
        currentPack = packInfo.packId;

        if (std::find(packQueue.begin(), packQueue.end(), packInfo.packId) == packQueue.end())
            packQueue.push_back(packInfo.packId);
    }
    else if (std::find(packQueue.begin(), packQueue.end(), packInfo.packId) == packQueue.end())
    {
        packQueue.push_back(packInfo.packId);
    }

    processChunkQueue();
    return true;
}

void NetworkSession::processChunkQueue()
{
    if (queueProcessing)
        return;

    queueProcessing = true;

    try
    {
        while (!pendingChunks.empty())
        {
            auto [packId, idx] = pendingChunks.front();
            pendingChunks.pop_front();

            auto packIter = loadedPacks.find(packId);
            if (packIter == loadedPacks.end())
                continue;

            auto& pack = packIter->second;
            if (idx < 0 || idx >= pack->chunkCount)
            {
                markedForDisconnect = true;
                queueProcessing = false;
                return;
            }

            if (pack->sent[static_cast<size_t>(idx)])
                continue;

            size_t startOffset = static_cast<size_t>(idx) * pack->maxChunkSize;
            size_t chunkSize = std::min(static_cast<size_t>(pack->maxChunkSize), pack->pack->getPackSize() - startOffset);

            std::string chunk = pack->pack->getPackChunkString(startOffset, chunkSize);
            if (chunk.empty())
            {
                markedForDisconnect = true;
                queueProcessing = false;
                return;
            }

            auto pkt = std::make_unique<proto::ResourcePackChunkDataPacket>();
            pkt->packId = pack->packId;
            pkt->packVersion = pack->pack->getPackVersion();
            pkt->chunkIndex = idx;
            pkt->progress = startOffset;
            pkt->data = std::move(chunk);

            send(std::move(pkt), true);

            pack->sent[static_cast<size_t>(idx)] = true;

            while (pack->nextToSend < pack->chunkCount && pack->sent[static_cast<size_t>(pack->nextToSend)])
                pack->nextToSend++;

            if (pack->nextToSend >= pack->chunkCount)
                nextPack();
        }
    } catch (...)
    {
        queueProcessing = false;
        throw;
    }

    queueProcessing = false;
}

void NetworkSession::nextPack()
{
    if (!packQueue.empty())
    {
        if (packQueue.front() == currentPack)
            packQueue.pop_front();
        else
            packQueue.erase(std::remove(packQueue.begin(), packQueue.end(), currentPack), packQueue.end());

        currentPack = packQueue.empty() ? uuid::UUID{} : packQueue.front();
    }
    else
    {
        currentPack = uuid::UUID{};
    }
}

} // namespace cyrex::nw::session
