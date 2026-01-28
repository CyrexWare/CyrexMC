#pragma once

#include "network/mcbe/compression/compressor.hpp"
#include "network/mcbe/compression/noop_compressor.hpp"
#include "network/mcbe/compression/zlib_compressor.hpp"
#include "network/mcbe/packet.hpp"
#include "network/mcbe/packet_pool.hpp"
#include "network/mcbe/packetbase.hpp"
#include "network/mcbe/transport.hpp"
#include "network/util/binary_stream.hpp"

#include <RakNet/RakNetTypes.h>
#include <memory>

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
        m_protocolId(0),
        m_compressor(std::make_unique<cyrex::network::mcbe::compression::NoopCompressor>())
    {
    }

    bool compressionEnabled = false;
    Phase phase = Phase::HANDSHAKE;

    void onRaw(RakNet::Packet packet, const uint8_t* data, size_t len);
    void send(cyrex::network::mcbe::PacketBase& packet);
    bool disconnectUserForIncompatiableProtocol(uint32_t);
    bool handleRequestNetworkSettings(cyrex::network::mcbe::PacketBase& packet);

    void setCompressor(std::unique_ptr<cyrex::network::mcbe::compression::Compressor> compressor);

    cyrex::network::mcbe::compression::Compressor& compressor() const
    {
        return *m_compressor;
    }

    void setProtocolId(std::uint32_t protocolId)
    {
        m_protocolId = protocolId;
    }

    std::uint32_t protocolId() const
    {
        return m_protocolId;
    }

    RakNet::RakNetGUID guid() const
    {
        return m_guid;
    }

    RakNet::SystemAddress address() const
    {
        return m_address;
    }

private:
    RakNet::RakNetGUID m_guid;
    RakNet::SystemAddress m_address;
    cyrex::network::mcbe::Transport* m_transport;

    std::uint32_t m_protocolId;
    std::unique_ptr<cyrex::network::mcbe::compression::Compressor> m_compressor;
};
} // namespace cyrex::network::session