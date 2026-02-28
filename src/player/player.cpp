#include "player.hpp"

#include "log/logging.hpp"
#include "network/mcbe/protocol/disconnect.hpp"
#include "network/mcbe/protocol/play_status.hpp"
#include "network/mcbe/protocol/protocol_info.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_data.hpp"
#include "network/mcbe/protocol/resource_pack_data_info.hpp"
#include "network/mcbe/protocol/resource_pack_stack.hpp"
#include "network/mcbe/protocol/resource_packs_info.hpp"
#include "network/session/network_session.hpp"
#include "server.hpp"

#include <algorithm>
#include <ranges>

#include <cmath>

namespace cyrex::player
{

Player::Player(SubClientId id, nw::session::NetworkSession* session, cyrex::Server& server) noexcept :
    m_subClientId(id),
    m_session(session),
    m_server(server)
{
}

Player::SubClientId Player::getSubClientId() const noexcept
{
    return m_subClientId;
}

void Player::sendPacket(std::unique_ptr<nw::protocol::Packet> packet, bool immediately)
{
    if (!m_session)
        return;
    packet->subClientId = m_subClientId;
    m_session->send(std::move(packet), immediately);
}

template <class... Packets>
void Player::sendPacketBatch(bool immediately, Packets&&... packets)
{
    if (!m_session)
        return;

    ((packets->subClientId = m_subClientId), ...);
    m_session->sendBatch(immediately, std::forward<Packets>(packets)...);
}

void Player::markForDisconnect() noexcept
{
    m_markedForDisconnect = true;
}

bool Player::isMarkedForDisconnect() const noexcept
{
    return m_markedForDisconnect;
}

void Player::doLoginSuccess()
{
    auto playStatus = std::make_unique<cyrex::nw::protocol::PlayStatusPacket>();
    playStatus->status = cyrex::nw::protocol::PlayStatus::LoginSuccess;

    auto resourcePacksInfo = std::make_unique<cyrex::nw::protocol::ResourcePacksInfoPacket>();
    const auto& defs = m_server.getResourcePackFactory().getResourceStack();
    resourcePacksInfo->resourcePackEntries.reserve(defs.size());

    for (const auto& defPtr : defs)
    {
        if (!defPtr)
            continue;
        const auto& def = *defPtr;
        int chunkCount = static_cast<int>(
            std::ceil(double(def.getPackSize()) / m_server.getResourcePackFactory().getMaxChunkSize()));
        logging::log("chunk={}", chunkCount);

        auto& entry = resourcePacksInfo->resourcePackEntries.emplace_back();
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
    }

    resourcePacksInfo->mustAccept = m_server.shouldForceResources();
    resourcePacksInfo->disableVibrantVisuals = false;
    resourcePacksInfo->worldTemplateId = uuid::randomUUID();
    resourcePacksInfo->worldTemplateVersion = "";

    sendPacketBatch(true, std::move(playStatus), std::move(resourcePacksInfo));
}

bool Player::handleResourcePackClientResponse(const nw::protocol::ResourcePackClientResponsePacket& pk)
{
    using namespace nw::protocol;

    switch (pk.responseStatus)
    {
        case ResourcePackClientResponseStatus::Refused:
            logging::warn("client refused resource packs");
            if (m_server.shouldForceResources())
                m_markedForDisconnect = true;
            break;

        case ResourcePackClientResponseStatus::SendPacks:
            for (const auto& entry : pk.packEntries)
            {
                auto resourcePack = m_server.getResourcePackFactory().getPackById(entry.uuid);
                if (!resourcePack)
                {
                    m_markedForDisconnect = true;
                    return true;
                }

                const int maxChunkSize = m_server.getResourcePackFactory().getMaxChunkSize();
                const int chunkCount = static_cast<int>(std::ceil(double(resourcePack->getPackSize()) / maxChunkSize));

                auto data = std::make_unique<nw::protocol::ResourcePackMeta>(resourcePack->getPackId(),
                                                                             resourcePack,
                                                                             maxChunkSize,
                                                                             chunkCount);

                m_packQueue.push_back(data->packId);
                m_loadedPacks.emplace(data->packId, std::move(data));

                auto dataInfoPkt = std::make_unique<nw::protocol::ResourcePackDataInfoPacket>();
                dataInfoPkt->packId = resourcePack->getPackId();
                dataInfoPkt->packVersion = resourcePack->getPackVersion();
                dataInfoPkt->maxChunkSize = maxChunkSize;
                dataInfoPkt->chunkCount = chunkCount;
                dataInfoPkt->compressedPackSize = resourcePack->getPackSize();
                dataInfoPkt->sha256 = resourcePack->getSha256();
                sendPacket(std::move(dataInfoPkt), false);
            }
            break;

        case ResourcePackClientResponseStatus::HaveAllPacks:
        {
            auto stackPkt = std::make_unique<nw::protocol::ResourcePackStackPacket>();
            stackPkt->mustAccept = m_server.shouldForceResources();
            const auto& defStack = m_server.getResourcePackFactory().getResourceStack();
            stackPkt->resourcePackStack.reserve(defStack.size());
            for (const auto& def : defStack)
            {
                auto& entry = stackPkt->resourcePackStack.emplace_back();
                entry.packId = uuid::toString(def->getPackId());
                entry.packVersion = def->getPackVersion();
                entry.subPackName = def->getSubPackName();
            }
            stackPkt->baseGameVersion = nw::protocol::ProtocolInfo::minecraftVersionNetwork;
            stackPkt->experiments = {};
            sendPacket(std::move(stackPkt), true);
            break;
        }

        case ResourcePackClientResponseStatus::Completed:
            logging::log("client has completed process");
            break;
    }

    return true;
}

bool Player::handleResourcePackChunkRequest(const nw::protocol::ResourcePackChunkRequestPacket& request)
{
    const nw::protocol::ResourcePackMeta* packInfoPtr = nullptr;
    auto packFinder = m_loadedPacks.find(request.packId);

    if (packFinder == m_loadedPacks.end())
    {
        const auto rawPack = m_server.getResourcePackFactory().getPackById(request.packId);
        if (!rawPack)
        {
            m_markedForDisconnect = true;
            return true;
        }

        const int maxSize = m_server.getResourcePackFactory().getMaxChunkSize();
        const int totalChunks = static_cast<int>((rawPack->getPackSize() + maxSize - 1) / maxSize);
        auto packInfo = std::make_unique<nw::protocol::ResourcePackMeta>(rawPack->getPackId(), rawPack, maxSize, totalChunks);
        packInfoPtr = m_loadedPacks.emplace(packInfo->packId, std::move(packInfo)).first->second.get();
    }
    else
        packInfoPtr = packFinder->second.get();

    auto& packInfo = *packInfoPtr;

    if (request.chunkIndex >= 0 && request.chunkIndex < packInfo.chunkCount)
        m_pendingChunks.emplace_back(packInfo.packId, static_cast<int>(request.chunkIndex));

    if (m_currentPack.is_nil())
        m_currentPack = packInfo.packId;
    if (std::ranges::find(m_packQueue, packInfo.packId) == m_packQueue.end())
        m_packQueue.push_back(packInfo.packId);

    processChunkQueue();
    return true;
}

void Player::processChunkQueue()
{
    if (m_queueProcessing)
        return;
    m_queueProcessing = true;

    try
    {
        while (!m_pendingChunks.empty())
        {
            auto [packId, idx] = m_pendingChunks.front();
            m_pendingChunks.pop_front();

            auto packIter = m_loadedPacks.find(packId);
            if (packIter == m_loadedPacks.end())
                continue;

            auto& pack = packIter->second;
            if (idx < 0 || idx >= pack->chunkCount)
            {
                m_markedForDisconnect = true;
                m_queueProcessing = false;
                return;
            }
            if (pack->sent.at(static_cast<size_t>(idx)))
                continue;

            const size_t startOffset = static_cast<size_t>(idx) * pack->maxChunkSize;
            const size_t chunkSize = std::min(static_cast<size_t>(pack->maxChunkSize),
                                              pack->pack->getPackSize() - startOffset);

            std::string chunk = pack->pack->getPackChunkString(startOffset, chunkSize);
            if (chunk.empty())
            {
                m_markedForDisconnect = true;
                m_queueProcessing = false;
                return;
            }

            auto pkt = std::make_unique<nw::protocol::ResourcePackChunkDataPacket>();
            pkt->packId = pack->packId;
            pkt->packVersion = pack->pack->getPackVersion();
            pkt->chunkIndex = idx;
            pkt->progress = startOffset;
            pkt->data = std::move(chunk);

            sendPacket(std::move(pkt), true);
            pack->sent.at(static_cast<size_t>(idx)) = true;

            while (pack->nextToSend < pack->chunkCount && pack->sent.at(static_cast<size_t>(pack->nextToSend)))
                pack->nextToSend++;

            if (pack->nextToSend >= pack->chunkCount)
                m_currentPack = uuid::UUID{};
        }
    } catch (...)
    {
        m_queueProcessing = false;
        throw;
    }

    m_queueProcessing = false;
}

void Player::nextPack()
{
    if (!m_packQueue.empty())
    {
        if (m_packQueue.front() == m_currentPack)
            m_packQueue.pop_front();
        else
            m_packQueue.erase(std::ranges::remove(m_packQueue, m_currentPack).begin(), m_packQueue.end());

        m_currentPack = m_packQueue.empty() ? uuid::UUID{} : m_packQueue.front();
    }
    else
    {
        m_currentPack = uuid::UUID{};
    }
}

void Player::disconnect(const std::string& message)
{
    auto packet = std::make_unique<nw::protocol::DisconnectPacket>();
    packet->message = message;
    sendPacket(std::move(packet), true);
}
} // namespace cyrex::player