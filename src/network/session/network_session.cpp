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
#include "network/mcbe/protocol/server_to_client_handshake.hpp"
#include "network/mcbe/protocol/types/login/LoginData.hpp"
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
#include <jwt-cpp/base.h>
#include <jwt-cpp/jwt.h>
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
        cyrex::logging::info(LOG_MCBE, "packet length = {}", packetLength);
        cyrex::logging::info(LOG_MCBE,
                             "packet id = {}0x{:02X} ({})",
                             logging::Color::GOLD,
                             packetId,
                             cyrex::nw::protocol::toSimpleName(cyrex::nw::protocol::makePacketId(packetId)));

        if (packetId != 0x01)
        {
            std::stringstream ss;
            const auto* payload = data + in.offset - packetLength + packetBuffer.offset;
            const size_t payloadSize = packetLength - (packetBuffer.offset);
            ss << "raw payload (" << payloadSize << " bytes): ";
            for (size_t i = 0; i < payloadSize; ++i)
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(payload[i]) << " ";
            cyrex::logging::info(LOG_MCBE, "{}", ss.str());
        }

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
                         ? protocol::PlayStatus::LoginFailedClient
                         : protocol::PlayStatus::LoginFailedServer;

    send(std::move(packet), true);
    return true;
}

void NetworkSession::send(std::unique_ptr<protocol::Packet> packet, const bool immediately)
{
    cyrex::logging::info("queueing packet with id = {}0x{:02X} ({})",
                         logging::Color::GOLD,
                         packet->getDef().networkId,
                         cyrex::nw::protocol::toSimpleName(protocol::makePacketId(packet->getDef().networkId)));
    if (immediately)
    {
        BinaryWriter packetBuffer;

        packet->encode(packetBuffer);

        sendInternal(packetBuffer);
        return;
    }
    m_sendQueue.push_back(std::move(packet));
}

template<typename... Packets>
void NetworkSession::sendBatch(const bool immediately, Packets&&... packets)
{
    std::vector<std::unique_ptr<protocol::Packet>> batch;
    batch.reserve(sizeof...(packets));
    (batch.push_back(std::forward<Packets>(packets)), ...);
    if (immediately)
    {
        BinaryWriter packetBuffer;
        for (const auto& packet : batch)
        {
            packet->encode(packetBuffer);
        }
        sendInternal(packetBuffer);
        return;
    }

    m_sendQueue.reserve(m_sendQueue.size() + batch.size());
    for (auto& packet : batch)
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
    std::vector<uint8_t> out;

    if (!compressionEnabled)
    {
        out = payload.getBuffer();
    }
    else
    {
        const auto* comp = protocol::getCompressor(compressor);
        if (comp && comp->shouldCompress(payload.size()))
        {
            std::vector<uint8_t> compressed = *comp->compress(payload.getBuffer());

            out.push_back(std::to_underlying(compressor));
            logging::info("compression success");

            out.insert(out.end(), compressed.begin(), compressed.end());
        }
        else
        {
            out.push_back(std::to_underlying(protocol::CompressionAlgorithm::NONE));
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

    m_transport->send(m_guid, out.data(), out.size());
}

static bool verifyChainData(const std::string& chain, std::string& identityPublicKeyDer, const long index)
{
    try{
        const auto jwtChain = jwt::decode(chain);
        const auto x5u = jwtChain.get_header_claim("x5u").as_string();
        const auto es384 = jwt::algorithm::es384{"-----BEGIN PUBLIC KEY-----\n" + x5u + "\n-----END PUBLIC KEY-----"};
        if (!identityPublicKeyDer.empty() && x5u != identityPublicKeyDer)
            return false;
        [&]() {
            if (index == 0)
                return jwt::verify().allow_algorithm(es384);
            if (index == 3)
                return jwt::verify().leeway(UINT32_MAX).allow_algorithm(es384);
            return jwt::verify().with_issuer("Mojang").allow_algorithm(es384);
        }().verify(jwtChain);
        if (index == 3)
        {
            return true;
        }
        if (!jwtChain.has_payload_claim("identityPublicKey"))
        {
            return false;
        }
        identityPublicKeyDer = jwtChain.get_payload_claim("identityPublicKey").as_string();
    }
    catch (const std::exception& e)
    {
        cyrex::logging::error(LOG_MCBE, "failed exception %s", e.what());
        return false;
    }
    return true;
}

static std::string createEncryptionJwt(protocol::AesEncryptor::EccKey* serverKey, const std::array<uint8_t, 16>& salt)
{
    std::vector<byte> derBuffer(1024);
    auto derLength = static_cast<word32>(derBuffer.size());
    int const ret = wc_EccPublicKeyToDer(serverKey, derBuffer.data(), derLength, 1);
    if (ret <= 0)
    {
        throw std::runtime_error("Failed to export public key to DER");
    }
    derLength = static_cast<word32>(ret);
    const std::string x5u = jwt::base::encode<jwt::alphabet::base64>(std::string(reinterpret_cast<char*>(derBuffer.data()), derLength));
    const std::string saltBase64 = jwt::base::encode<jwt::alphabet::base64>(std::string(reinterpret_cast<const char*>(salt.data()), salt.size()));
    auto builder = jwt::create()
            .set_header_claim("alg", jwt::claim(std::string("ES384")))
            .set_header_claim("x5u", jwt::claim(x5u))
            .set_payload_claim("salt", jwt::claim(saltBase64))
            .sign(jwt::algorithm::none());
    std::string message = builder.substr(0, builder.size() - 1);
    WC_RNG rng;
    if (wc_InitRng(&rng) != 0) throw std::runtime_error("RNG Init Failed");
    std::array<byte, 48> hash{};
    wc_Sha384 sha;
    wc_InitSha384(&sha);
    wc_Sha384Update(&sha, reinterpret_cast<const byte*>(message.data()), message.size());
    wc_Sha384Final(&sha, hash.data());
    std::vector<byte> derSig(200);
    auto derSigLen = static_cast<word32>(derSig.size());
    if (wc_ecc_sign_hash(hash.data(), (word32)hash.size(), derSig.data(), &derSigLen, &rng, serverKey) != 0) {
        wc_FreeRng(&rng);
        throw std::runtime_error("ECC Sign Failed");
    }
    wc_FreeRng(&rng);
    mp_int r;
    mp_int s;
    if (mp_init_multi(&r, &s, nullptr, nullptr, nullptr, nullptr) != MP_OKAY) {
        throw std::runtime_error("MP Init Failed");
    }
    std::string signatureRaw;
    if (DecodeECC_DSA_Sig(derSig.data(), derSigLen, &r, &s) == 0) {
        std::array<byte, 96> rawBuffer{};
        int const rSize = mp_unsigned_bin_size(&r);
        int const sSize = mp_unsigned_bin_size(&s);
        mp_to_unsigned_bin(&r, rawBuffer.data() + (48 - rSize));
        mp_to_unsigned_bin(&s, rawBuffer.data() + 48 + (48 - sSize));
        signatureRaw = std::string(reinterpret_cast<char*>(rawBuffer.data()), rawBuffer.size());
    }
    mp_clear(&r);
    mp_clear(&s);
    if (signatureRaw.empty()) throw std::runtime_error("Signature decoding failed");
    return message + "." + jwt::base::encode<jwt::alphabet::base64url>(signatureRaw);
}

bool NetworkSession::verifyLegacyJwtChains(const std::string& chainData, const std::string& clientDataJwt, const bool isOnline)
{
    const auto chains = nlohmann::json::parse(chainData).at("chain").get<std::vector<std::string>>();
    if (chains.size() != 3 && isOnline)
    {
        return false;
    }
    std::string identityPublicKeyDer{};
    for (auto [i, chain] : chains | std::views::enumerate)
    {
        if(!verifyChainData(chain, identityPublicKeyDer, i))
        {
            return false;
        }
    }
    if(!verifyChainData(clientDataJwt, identityPublicKeyDer, 3))
    {
        return false;
    }
    std::string const playerPublicKey = jwt::base::decode<jwt::alphabet::base64>(identityPublicKeyDer);
    m_cipher = protocol::AesEncryptor(m_server.getServerPrivateKey(), playerPublicKey);
    std::string const payload = createEncryptionJwt(m_cipher->serverKey, m_cipher->salt);
    auto pS2Cjwt = std::make_unique<protocol::ServerToClientHandshakePacket>();
    pS2Cjwt->jwt = payload;
    send(std::move(pS2Cjwt), true);
    encryptionEnabled = true;
    std::cout << hexDump(m_cipher->key.data(), 32) << std::endl;
    return true;
}

bool NetworkSession::handleLogin(const uint32_t version, const std::string& authInfoJson, const std::string& clientDataJwt)
{
    if (!protocol::isSupportedProtocol(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }
    const auto authData = nlohmann::json::parse(authInfoJson);
    protocol::AuthenticationInfo const auth = authData.get<protocol::AuthenticationInfo>();
    if (cyrex::Server::isEncryptionEnabled())
    {
        if (!verifyLegacyJwtChains(auth.Certificate, clientDataJwt, cyrex::Server::isOnlineMode()))
        {
            //TODO: kick
            return false;
        }
    }else{
        doLoginSuccess();
    }
    return true;
}

bool NetworkSession::handleClientToServerHandshake()
{
    doLoginSuccess();
    return true;
}

void NetworkSession::doLoginSuccess()
{
    constexpr std::vector<std::unique_ptr<Packet>> packets;
    auto playStatus = std::make_unique<protocol::PlayStatusPacket>();
    playStatus->status = protocol::PlayStatus::LoginSuccess;

    auto resourcePacksInfo = std::make_unique<protocol::ResourcePacksInfoPacket>();
    const auto& defs = m_server.getResourcePackFactory().getResourceStack();
    resourcePacksInfo->resourcePackEntries.reserve(defs.size());

    for (const auto& defPtr : defs)
    {
        if (!defPtr)
            continue;

        const auto& def = *defPtr;

        int chunkCount = static_cast<int>(
            std::ceil(static_cast<double>(def.getPackSize()) / m_server.getResourcePackFactory().getMaxChunkSize()));

        logging::log("chunk={}", chunkCount);

        auto & [packId,
               packVersion,
               packSize,
               encryptionKey,
               subPackName,
               contentIdentity,
               scripting,
               addonPack,
               raytracingCapable,
               cdnUrl] = resourcePacksInfo->resourcePackEntries.emplace_back();

        packId = def.getPackId();
        packVersion = def.getPackVersion();
        packSize = def.getPackSize();
        encryptionKey = def.getEncryptionKey();
        subPackName = "";
        contentIdentity = uuid::toString(packId);
        scripting = def.usesScript();
        addonPack = def.isAddonPack();
        raytracingCapable = def.isRaytracingCapable();
        cdnUrl = def.cdnUrl();
    }

    resourcePacksInfo->mustAccept = m_server.shouldForceResources();
    resourcePacksInfo->disableVibrantVisuals = false;

    resourcePacksInfo->worldTemplateId = uuid::randomUUID();
    resourcePacksInfo->worldTemplateVersion = "";
    sendBatch(
        true,
        std::move(playStatus),
        std::move(resourcePacksInfo)
    );
}

bool NetworkSession::handleRequestNetworkSettings(const uint32_t version)
{
    if (!protocol::isSupportedProtocol(version))
    {
        disconnectUserForIncompatibleProtocol(version);
        return false;
    }

    compressor = protocol::CompressionAlgorithm::ZLIB;

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

bool NetworkSession::handleResourcePackClientResponse(const protocol::ResourcePackClientResponsePacket& pk)
{
    using protocol::ResourcePackClientResponseStatus;

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
                    markedForDisconnect = true;
                    return true;
                }

                int const maxChunkSize = m_server.getResourcePackFactory().getMaxChunkSize();
                int const chunkCount = static_cast<int>(
                    std::ceil(static_cast<double>(resourcePack->getPackSize()) / maxChunkSize));

                auto data = std::make_unique<protocol::ResourcePackMeta>(resourcePack->getPackId(),
                                                                         resourcePack,
                                                                         maxChunkSize,
                                                                         chunkCount);

                m_packQueue.push_back(data->packId);
                m_loadedPacks.emplace(data->packId, std::move(data));

                auto dataInfoPkt = std::make_unique<protocol::ResourcePackDataInfoPacket>();
                dataInfoPkt->packId = resourcePack->getPackId();
                dataInfoPkt->packVersion = resourcePack->getPackVersion();
                dataInfoPkt->maxChunkSize = maxChunkSize;
                dataInfoPkt->chunkCount = chunkCount;
                dataInfoPkt->compressedPackSize = resourcePack->getPackSize();
                dataInfoPkt->sha256 = resourcePack->getSha256();
                send(std::move(dataInfoPkt), false);
            }
            break;
        }

        case ResourcePackClientResponseStatus::HaveAllPacks:
        {
            auto stackPkt = std::make_unique<protocol::ResourcePackStackPacket>();
            stackPkt->mustAccept = m_server.shouldForceResources();

            const auto& defStack = m_server.getResourcePackFactory().getResourceStack();
            stackPkt->resourcePackStack.reserve(defStack.size());

            for (const auto& def : defStack)
            {
                auto &[packId, packVersion, subPackName] = stackPkt->resourcePackStack.emplace_back();
                packId = uuid::toString(def->getPackId());
                packVersion = def->getPackVersion();
                subPackName = def->getSubPackName();
            }

            stackPkt->baseGameVersion = protocol::ProtocolInfo::minecraftVersionNetwork;
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

bool NetworkSession::handleResourcePackChunkRequest(const cyrex::nw::protocol::ResourcePackChunkRequestPacket& request)
{
    protocol::ResourcePackMeta* packInfoPtr = nullptr;

    auto packFinder = m_loadedPacks.find(request.packId);

    if (packFinder == m_loadedPacks.end())
    {
        const auto rawPack = m_server.getResourcePackFactory().getPackById(request.packId);
        if (!rawPack)
        {
            markedForDisconnect = true;
            return true;
        }

        int maxSize = m_server.getResourcePackFactory().getMaxChunkSize();
        int totalChunks = static_cast<int>((rawPack->getPackSize() + maxSize - 1) / maxSize);

        auto packInfo = std::make_unique<protocol::ResourcePackMeta>(rawPack->getPackId(), rawPack, maxSize, totalChunks);

        packInfoPtr = m_loadedPacks.emplace(packInfo->packId, std::move(packInfo)).first->second.get();
    }
    else
    {
        packInfoPtr = packFinder->second.get();
    }

    auto& packInfo = *packInfoPtr;

    if (request.chunkIndex >= 0 && request.chunkIndex < packInfo.chunkCount)
    {
        // packInfo.want[static_cast<size_t>(request.chunkIndex)] = true;

        m_pendingChunks.emplace_back(packInfo.packId, static_cast<int>(request.chunkIndex));
    }

    if (m_currentPack.is_nil())
    {
        m_currentPack = packInfo.packId;

        if (std::ranges::find(m_packQueue, packInfo.packId) == m_packQueue.end())
            m_packQueue.push_back(packInfo.packId);
    }
    else if (std::ranges::find(m_packQueue, packInfo.packId) == m_packQueue.end())
    {
        m_packQueue.push_back(packInfo.packId);
    }

    processChunkQueue();
    return true;
}

void NetworkSession::processChunkQueue()
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

            const auto& pack = packIter->second;
            if (idx < 0 || idx >= pack->chunkCount)
            {
                markedForDisconnect = true;
                m_queueProcessing = false;
                return;
            }

            if (pack->sent[static_cast<size_t>(idx)])
                continue;

            const size_t startOffset = static_cast<size_t>(idx) * pack->maxChunkSize;
            const size_t chunkSize = std::min(static_cast<size_t>(pack->maxChunkSize), pack->pack->getPackSize() - startOffset);

            std::string chunk = pack->pack->getPackChunkString(startOffset, chunkSize);
            if (chunk.empty())
            {
                markedForDisconnect = true;
                m_queueProcessing = false;
                return;
            }

            auto pkt = std::make_unique<cyrex::nw::protocol::ResourcePackChunkDataPacket>();
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
        m_queueProcessing = false;
        throw;
    }

    m_queueProcessing = false;
}

void NetworkSession::nextPack()
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

} // namespace cyrex::nw::session
