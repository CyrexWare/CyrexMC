#include "network_session.hpp"

#include "log/logging.hpp"
#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/compressors.hpp"
#include "network/mcbe/packetids.hpp"
#include "network/mcbe/protocol/disconnect.hpp"
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
#include "network/mcbe/protocol/types/ViolationSeverity.hpp"
#include "network/mcbe/protocol/types/login/LoginData.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackInfoEntry.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackMeta.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackStackEntry.hpp"
#include "network/mcbe/resourcepacks/resource_pack_def.hpp"
#include "network/raknet/handler/raknet_handler.hpp"
#include "player/player.hpp"
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
#include <wolfssl/options.h>

#include <cstdint>

#ifdef min
#undef min
#endif

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
namespace AnsiColor
{
const auto reset = "\033[0m";
const auto green = "\033[32m";
const auto red = "\033[31m";
const auto yellow = "\033[33m";
const auto blue = "\033[34m";
} // namespace AnsiColor

void debugByteBuffer(const std::vector<uint8_t>& buffer)
{
    if (buffer.empty())
        return;
    auto& out = std::cout;
    const auto originalFlags = out.flags();
    const auto originalFill = out.fill();
    out << "           +-------------------------------------------------+\n";
    out << "  Hex View |  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F |   ASCII View\n";
    out << "+----------+-------------------------------------------------+----------------+\n";
    for (size_t i = 0; i < buffer.size(); i += 16)
    {
        out << "| " << std::setw(8) << std::setfill('0') << std::hex << std::uppercase << i << " | ";
        out.fill(' ');
        for (int j = 0; j < 16; ++j)
        {
            if (i + j < buffer.size())
            {
                const uint8_t byte = buffer.at(i + j);
                out << (isprint(byte) ? AnsiColor::green : AnsiColor::red) << std::setw(2) << std::setfill('0')
                    << std::hex << std::uppercase << static_cast<int>(byte) << AnsiColor::reset << " ";
            }
            else
                out << AnsiColor::yellow << "--" << AnsiColor::reset << " ";
        }
        out.fill(' ');
        out << "|";
        for (int j = 0; j < 16; ++j)
        {
            if (i + j < buffer.size())
            {
                if (const char c = static_cast<char>(buffer.at(i + j)); isprint(c))
                    out << AnsiColor::blue << c << AnsiColor::reset;
                else
                    out << ".";
            }
            else
                out << " ";
        }
        out << "|\n";
    }
    out << "+----------+-------------------------------------------------+----------------+\n";
    out.flags(originalFlags);
    out.fill(originalFill);
}
} // namespace

namespace cyrex::network::session
{

void NetworkSession::tick()
{
    // tick all the players
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
                      protocol::toReadablePacketName(network::protocol::makePacketId(packetId)));

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
            logging::error(LOG_MCBE, "unknown packet id 0x{:02x}", packetId);
            return;
        }

        const auto packet = packetDef->decode(packetBuffer);
        if (!packet)
        {
            logging::error(LOG_MCBE, "error decoding packet");
            return;
        }
        packet->subClientId = static_cast<protocol::SubClientId>(packetHeader >> 10 & 0x03);
        if (!packet->handle(*this))
        {
            logging::error(LOG_MCBE, "error handling packet");
            return;
        }
    } while (in.remaining() > 0);
}

bool NetworkSession::disconnectUserForIncompatibleProtocol(const uint32_t protocolVersion)
{
    auto packet = std::make_unique<network::protocol::PlayStatusPacket>();
    packet->status = protocolVersion < network::protocol::ProtocolInfo::currentProtocol
                         ? network::protocol::PlayStatus::LoginFailedClient
                         : network::protocol::PlayStatus::LoginFailedServer;

    send(std::move(packet), true);
    return true;
}

void NetworkSession::send(std::unique_ptr<protocol::Packet> packet, const bool immediately)
{
    logging::info("queueing packet with id = {}0x{:02X} ({})",
                  logging::Color::GOLD,
                  packet->getDef().networkId,
                  protocol::toReadablePacketName(protocol::makePacketId(packet->getDef().networkId)));
    if (immediately)
    {
        BinaryWriter packetBuffer;

        packet->encode(packetBuffer);

        sendInternal(packetBuffer);
        return;
    }
    m_sendQueue.push_back(std::move(packet));
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
    try
    {
        const auto jwtChain = jwt::decode(chain);
        const auto x5u = jwtChain.get_header_claim("x5u").as_string();
        const auto es384 = jwt::algorithm::es384{"-----BEGIN PUBLIC KEY-----\n" + x5u + "\n-----END PUBLIC KEY-----"};
        if (!identityPublicKeyDer.empty() && x5u != identityPublicKeyDer)
            return false;
        [&]()
        {
            if (index == 0)
                return jwt::verify().allow_algorithm(es384);
            if (index == 3)
                return jwt::verify().leeway(UINT32_MAX).allow_algorithm(es384);
            return jwt::verify().with_issuer("Mojang").allow_algorithm(es384);
        }()
            .verify(jwtChain);
        if (index == 3)
        {
            return true;
        }
        if (!jwtChain.has_payload_claim("identityPublicKey"))
        {
            return false;
        }
        identityPublicKeyDer = jwtChain.get_payload_claim("identityPublicKey").as_string();
    } catch (const std::exception& e)
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
    const int ret = wc_EccPublicKeyToDer(serverKey, derBuffer.data(), derLength, 1);
    if (ret <= 0)
    {
        throw std::runtime_error("Failed to export public key to DER");
    }
    derLength = static_cast<word32>(ret);
    const std::string x5u = jwt::base::encode<jwt::alphabet::base64>(
        std::string(reinterpret_cast<char*>(derBuffer.data()), derLength));
    const std::string saltBase64 = jwt::base::encode<jwt::alphabet::base64>(
        std::string(reinterpret_cast<const char*>(salt.data()), salt.size()));
    auto builder = jwt::create()
                       .set_header_claim("alg", jwt::claim(std::string("ES384")))
                       .set_header_claim("x5u", jwt::claim(x5u))
                       .set_payload_claim("salt", jwt::claim(saltBase64))
                       .sign(jwt::algorithm::none());
    builder.pop_back();
    WC_RNG rng;
    if (wc_InitRng(&rng) != 0)
    {
        throw std::runtime_error("RNG Init Failed");
    }
    std::array<byte, 48> hash{};
    wc_Sha384 sha;
    wc_InitSha384(&sha);
    wc_Sha384Update(&sha, reinterpret_cast<const byte*>(builder.data()), builder.size());
    wc_Sha384Final(&sha, hash.data());
    std::vector<byte> derSig(200);
    auto derSigLen = static_cast<word32>(derSig.size());
    if (wc_ecc_sign_hash(hash.data(), (word32)hash.size(), derSig.data(), &derSigLen, &rng, serverKey) != 0)
    {
        wc_FreeRng(&rng);
        throw std::runtime_error("ECC Sign Failed");
    }
    wc_FreeRng(&rng);
    mp_int r;
    mp_int s;
    if (mp_init_multi(&r, &s, nullptr, nullptr, nullptr, nullptr) != MP_OKAY)
    {
        throw std::runtime_error("MP Init Failed");
    }
    std::string signatureRaw;
    if (DecodeECC_DSA_Sig(derSig.data(), derSigLen, &r, &s) == 0)
    {
        std::array<byte, 96> rawBuffer{};
        const int rSize = mp_unsigned_bin_size(&r);
        const int sSize = mp_unsigned_bin_size(&s);
        mp_to_unsigned_bin(&r, rawBuffer.data() + (48 - rSize));
        mp_to_unsigned_bin(&s, rawBuffer.data() + 48 + (48 - sSize));
        signatureRaw = std::string(reinterpret_cast<char*>(rawBuffer.data()), rawBuffer.size());
    }
    mp_clear(&r);
    mp_clear(&s);
    if (signatureRaw.empty())
    {
        throw std::runtime_error("Signature decoding failed");
    }
    return builder + "." + jwt::base::encode<jwt::alphabet::base64url>(signatureRaw);
}

bool NetworkSession::verifyLegacyJwtChains(const std::string& chainData,
                                           const std::string& clientDataJwt,
                                           const bool isOnline,
                                           const bool isEncryption)
{
    const auto chains = nlohmann::json::parse(chainData).at("chain").get<std::vector<std::string>>();
    if (chains.size() != 3 && isOnline)
    {
        return false;
    }
    std::string identityPublicKeyDer{};
    for (auto [i, chain] : chains | std::views::enumerate)
    {
        if (!verifyChainData(chain, identityPublicKeyDer, i))
        {
            return false;
        }
    }
    if (!verifyChainData(clientDataJwt, identityPublicKeyDer, 3))
    {
        return false;
    }
    if (encryptionEnabled || !isEncryption)
    {
        return true;
    }
    const std::string playerPublicKey = jwt::base::decode<jwt::alphabet::base64>(identityPublicKeyDer);
    m_cipher = protocol::AesEncryptor(m_server.getServerPrivateKey(), playerPublicKey);
    try
    {
        const std::string payload = createEncryptionJwt(m_cipher->serverKey, m_cipher->salt);
        auto serverToClientHandshake = std::make_unique<protocol::ServerToClientHandshakePacket>();
        serverToClientHandshake->jwt = payload;
        send(std::move(serverToClientHandshake), true);
        encryptionEnabled = true;
        std::cout << hexDump(m_cipher->key.data(), 32) << std::endl;
    } catch (const std::exception& e)
    {
        cyrex::logging::error(LOG_MCBE, "failed exception %s", e.what());
        return false;
    }
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
    const protocol::AuthenticationInfo auth = authData.get<protocol::AuthenticationInfo>();
    if (!verifyLegacyJwtChains(auth.Certificate, clientDataJwt, cyrex::Server::isOnlineMode(), Server::isEncryptionEnabled()))
    {
        //TODO: kick
        return false;
    }
    if (!encryptionEnabled)
    {
        auto& player = m_server.createPlayer(protocol::SubClientId::PrimaryClient, this);
        m_players[protocol::SubClientId::PrimaryClient] = &player;
        // not sure if we need this or not
        m_primaryLoggedIn = true;

        player.doLoginSuccess();
    }
    return true;
}

bool NetworkSession::handleSubClientLogin(const std::string& authInfoJson, const std::string& clientDataJwt) // i have no idea yet
{
    const auto authData = nlohmann::json::parse(authInfoJson);
    const protocol::AuthenticationInfo auth = authData.get<protocol::AuthenticationInfo>();
    if (!verifyLegacyJwtChains(auth.Certificate, clientDataJwt, cyrex::Server::isOnlineMode(), false))
    {
        //TODO: kick
        return false;
    }
    if (!encryptionEnabled)
    {
        // TODO: handle subclient logins
        //player.doLoginSuccess();
    }
    return true;
}

bool NetworkSession::handleClientToServerHandshake()
{
    if (!encryptionEnabled)
    {
        cyrex::logging::error(LOG_MCBE, "client sent ClientToServerHandshakePacket without encryption enabled");
        return false;
    }
    auto& player = m_server.createPlayer(protocol::SubClientId::PrimaryClient, this);
    m_players[protocol::SubClientId::PrimaryClient] = &player;
    // not sure if we need this or not
    m_primaryLoggedIn = true;
    player.doLoginSuccess();
    return true;
}

bool NetworkSession::handlePacketViolationWarning(const protocol::ViolationSeverity severity,
                                                  std::int32_t packetId,
                                                  std::string message)
{
    logging::error("PacketViolation: Severity > {} ID > 0x{:02X}({}) MSG > {}",
                   magic_enum::enum_name(severity),
                   packetId,
                   cyrex::network::protocol::toReadablePacketName(protocol::makePacketId(packetId)),
                   message);
    //m_markedForDisconnect = true;
    return true;
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

} // namespace cyrex::network::session
