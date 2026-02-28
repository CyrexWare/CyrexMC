#pragma once

#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/compressor.hpp"
#include "network/mcbe/encryption/encryption.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packet_factory.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_request.hpp"
#include "network/mcbe/protocol/resource_pack_client_response.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/mcbe/protocol/types/SubClientId.hpp"
#include "network/mcbe/protocol/types/ViolationSeverity.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackMeta.hpp"
#include "network/mcbe/transport.hpp"
#include "server.hpp"
#include "util/uuid.hpp"

#include <RakNet/RakNetTypes.h>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <uuid.h>

namespace cyrex::nw::protocol
{
class ResourcePackChunkRequestPacket;
}

namespace cyrex::player
{
class Player;
}

namespace cyrex::nw::session
{

// need getter/setter commands for this and proper usage
enum class Phase
{
    PRELOGIN,
    LOGIN,
    ENCRYPTION, // also handshake
    RESOURCE_PACKS,
    PRE_SPAWN,
    IN_GAME,
    DEAD
};

class NetworkSession
{
public:
    NetworkSession(const RakNet::RakNetGUID& guid,
                   const RakNet::SystemAddress& address,
                   protocol::Transport* transport,
                   Server& server) :
        m_guid(guid),
        m_address(address),
        m_transport(transport),
        m_server(server)
    {
        m_packetFactory.registerAll();
    }

    bool compressionEnabled = false;
    protocol::CompressionAlgorithm compressor = protocol::CompressionAlgorithm::NONE;

    bool encryptionEnabled = false;
    Phase phase = Phase::PRELOGIN;

    void onRaw(const RakNet::Packet& packet, const uint8_t* data, size_t len);
    void send(std::unique_ptr<protocol::Packet> packet, bool immediately = false);
    template <typename... Packets>
    void sendBatch(const bool immediately, Packets&&... packets)
    {
        std::vector<std::unique_ptr<protocol::Packet>> batch;
        batch.reserve(sizeof...(packets));
        (batch.push_back(std::forward<Packets>(packets)), ...);
        if (immediately)
        {
            io::BinaryWriter packetBuffer;
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

    void flush();
    bool disconnectUserForIncompatibleProtocol(const uint32_t protocolVersion);
    bool handleLogin(uint32_t version, const std::string& authInfoJson, const std::string& clientDataJwt);
    bool handleSubClientLogin(const std::string& authInfoJson, const std::string& clientDataJwt);
    bool handleClientToServerHandshake();
    bool handlePacketViolationWarning(protocol::ViolationSeverity severity, std::int32_t packetId, std::string message);
    bool handleRequestNetworkSettings(uint32_t version);
    void tick();

    void setProtocolId(const std::uint32_t protocolId)
    {
        m_protocolId = protocolId;
    }

    [[nodiscard]] std::uint32_t protocolId() const
    {
        return m_protocolId;
    }

    [[nodiscard]] RakNet::RakNetGUID guid() const
    {
        return m_guid;
    }

    [[nodiscard]] RakNet::SystemAddress address() const
    {
        return m_address;
    }

    [[nodiscard]] protocol::AesEncryptor& getEncryptor()
    {
        return *m_cipher;
    }

    [[nodiscard]] cyrex::player::Player* getPlayer(protocol::SubClientId id = protocol::SubClientId::PrimaryClient) noexcept
    {
        if (auto it = m_players.find(id); it != m_players.end())
            return it->second;

        return nullptr;
    }

private:
    void sendInternal(const io::BinaryWriter& payload);
    bool verifyLegacyJwtChains(const std::string& chainData, const std::string& clientDataJwt, bool isOnline, bool isEncryption);
    std::vector<std::unique_ptr<protocol::Packet>> m_sendQueue;

    RakNet::RakNetGUID m_guid;
    RakNet::SystemAddress m_address;
    protocol::Transport* m_transport;

    Server& m_server;

    std::uint32_t m_protocolId{0};
    std::optional<protocol::AesEncryptor> m_cipher;
    std::map<protocol::SubClientId, cyrex::player::Player*> m_players;
    bool m_primaryLoggedIn{false};
    protocol::PacketFactory m_packetFactory;
};
} // namespace cyrex::nw::session
