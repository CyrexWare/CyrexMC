#pragma once

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "network/mcbe/compression/compressor.hpp"
#include "network/mcbe/compression/noop_compressor.hpp"
#include "network/mcbe/compression/zlib_compressor.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_pool.hpp"
#include "network/mcbe/packetbase.hpp"
#include "network/mcbe/transport.hpp"

#include <RakNet/RakNetTypes.h>
#include <functional>
#include <memory>
#include <queue>

#include <cstdint>

namespace cyrex::network::session
{

enum class Phase
{
    HANDSHAKE,
    PLAY
};

class NetworkSession
{
public:
    NetworkSession(RakNet::RakNetGUID guid, RakNet::SystemAddress address, cyrex::network::mcbe::Transport* transport) :
        m_guid(guid),
        m_address(address),
        m_transport(transport),

        m_compressor(std::make_unique<cyrex::network::mcbe::compression::NoopCompressor>())
    {
    }

    bool compressionEnabled = false;
    Phase phase = Phase::HANDSHAKE;
    bool markedForDisconnect = false;

    void onRaw(const RakNet::Packet& packet, const uint8_t* data, size_t len);
    void send(cyrex::network::mcbe::PacketBase& packet, bool immediately = false);
    void flush();
    bool disconnectUserForIncompatiableProtocol(uint32_t);
    bool handleRequestNetworkSettings(cyrex::network::mcbe::PacketBase& packet);
    void tick();

    void setCompressor(std::unique_ptr<cyrex::network::mcbe::compression::Compressor> compressor);

    [[nodiscard]] cyrex::network::mcbe::compression::Compressor& compressor() const
    {
        return *m_compressor;
    }

    void setProtocolId(std::uint32_t protocolId)
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

private:
    void sendInternal(cyrex::network::mcbe::PacketBase& packet);
    std::queue<std::function<void()>> m_sendQueue;

    RakNet::RakNetGUID m_guid;
    RakNet::SystemAddress m_address;
    cyrex::network::mcbe::Transport* m_transport;

    std::uint32_t m_protocolId{0};
    std::unique_ptr<cyrex::network::mcbe::compression::Compressor> m_compressor;
};
} // namespace cyrex::network::session
