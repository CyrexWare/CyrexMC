#pragma once

#include "network/mcbe/compression/compressor.hpp"
#include "network/mcbe/encryption/encryption.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packet_factory.hpp"
#include "network/mcbe/protocol/resource_pack_chunk_request.hpp"
#include "network/mcbe/protocol/resource_pack_client_response.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackClientResponseStatus.hpp"
#include "network/mcbe/protocol/types/packs/ResourcePackData.hpp"
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
    cyrex::nw::protocol::CompressionAlgorithm compressor;

    bool encryptionEnabled = false;
    Phase phase = Phase::PRELOGIN;
    bool markedForDisconnect = false;

    void onRaw(const RakNet::Packet& packet, const uint8_t* data, size_t len);
    void send(std::unique_ptr<protocol::Packet> packet, bool immediately = false);
    void sendBatch(std::vector<std::unique_ptr<protocol::Packet>> packets, bool immediately = false);
    void flush();
    bool disconnectUserForIncompatibleProtocol(uint32_t);
    bool handleLogin(uint32_t version, const std::string& authInfoJson, const std::string& clientDataJwt);
    void doLoginSuccess();
    bool handleRequestNetworkSettings(uint32_t version);
    bool handleResourcePackClientResponse(const protocol::ResourcePackClientResponsePacket& pk);
    bool handleResourcePackChunkRequest(const protocol::ResourcePackChunkRequestPacket& pk);
    void nextPack();
    void processChunkQueue();
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

private:
    void sendInternal(const io::BinaryWriter& payload);
    std::vector<std::unique_ptr<protocol::Packet>> m_sendQueue;

    RakNet::RakNetGUID m_guid;
    RakNet::SystemAddress m_address;
    protocol::Transport* m_transport;

    Server& m_server;

    std::uint32_t m_protocolId{0};
    std::optional<protocol::AesEncryptor> m_cipher;

    protocol::PacketFactory m_packetFactory;

    std::map<util::UUID, std::shared_ptr<protocol::ResourcePackData>> loadedPacks;
    std::deque<util::UUID> packQueue;
    std::deque<std::pair<util::UUID, int>> pendingChunks;

    util::UUID currentPack{};
    bool queueProcessing = false;
};
} // namespace cyrex::nw::session
