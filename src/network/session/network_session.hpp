#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/compressor.hpp"
#include "network/mcbe/encryption/encryption.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_def.hpp"
#include "network/mcbe/packet_factory.hpp"
#include "network/mcbe/protocol/types/CompressionAlgorithm.hpp"
#include "network/mcbe/transport.hpp"

#include <RakNet/RakNetTypes.h>
#include <functional>
#include <memory>
#include <queue>

#include <cstdint>

namespace cyrex::nw::session
{

enum class Phase
{
    HANDSHAKE,
    PLAY
};

class NetworkSession
{
public:
    NetworkSession(const RakNet::RakNetGUID& guid, const RakNet::SystemAddress& address, protocol::Transport* transport) :
        m_guid(guid),
        m_address(address),
        m_transport(transport)
    {
        m_packetFactory.registerAll();
    }

    bool compressionEnabled = false;
    cyrex::nw::protocol::CompressionAlgorithm compressor;

    bool encryptionEnabled = false;
    Phase phase = Phase::HANDSHAKE;
    bool markedForDisconnect = false;

    void onRaw(const RakNet::Packet& packet, const uint8_t* data, size_t len);
    void send(std::unique_ptr<protocol::Packet> packet, bool immediately = false);
    void sendBatch(std::vector<std::unique_ptr<protocol::Packet>> packets, bool immediately = false);
    void flush();
    bool disconnectUserForIncompatibleProtocol(uint32_t);
    bool handleLogin(uint32_t version, const std::string& authInfoJson, const std::string& clientDataJwt);
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

private:
    void sendInternal(const io::BinaryWriter& payload);
    std::vector<std::unique_ptr<protocol::Packet>> m_sendQueue;

    RakNet::RakNetGUID m_guid;
    RakNet::SystemAddress m_address;
    protocol::Transport* m_transport;

    std::uint32_t m_protocolId{0};
    std::optional<protocol::AesEncryptor> m_cipher;

    protocol::PacketFactory m_packetFactory;
};
} // namespace cyrex::nw::session
