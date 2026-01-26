#include "server.hpp"

#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <vector>
#include <span>

// Raknet
#include "RakNet/BitStream.h"
#include "RakNet/MessageIdentifiers.h"

#include <RakNet/MessageIdentifiers.h>
#include <RakNet/RakSleep.h>

#include <zlib.h>

void writeVarInt32u(RakNet::BitStream& bs, std::uint32_t value)
{
    while (value > 0x7F)          // While more than 7 bits remain
    {
        // Write the lower 7 bits with the continuation bit set (0x80)
        bs.Write(std::uint8_t(value & 0x7F | 0x80));
        value >>= 7; // Shift right by 7 bits
    }
    // Write the last byte with the continuation bit unset
    bs.Write(std::uint8_t(value));
}

static std::uint32_t readVarInt32u(RakNet::BitStream& bs)
{
    std::uint32_t value{};
    std::uint32_t shift{};
    std::uint8_t b;
    do
    {
        bs.Read<std::uint8_t>(b);

        // Take the lower 7 bits and shift them into position
        value |= (b & 0x7F) << shift;
        shift += 7;
    } 
    while ((b & 0x80) != 0); // Check the continuation bit (MSB)

    return value;
}
void DumpPacketHex(RakNet::Packet* packet)
{
    std::print("Packet from {}:{}, length={}\n",
               packet->systemAddress.ToString(false),
               packet->systemAddress.GetPort(),
               packet->length);

    for (unsigned int i = 0; i < packet->length; i++)
    {
        std::print("0x{:02X} ", static_cast<unsigned char>(packet->data[i]));
        if ((i + 1) % 16 == 0)
            std::print("\n");
    }
    std::print("\n");
}

void handleGamePacket(RakNet::RakPeerInterface* peer, RakNet::Packet* packet)
{
    RakNet::BitStream in(packet->data, packet->length, false);
    in.IgnoreBytes(1);

    std::uint32_t length = readVarInt32u(in);
    std::uint32_t id = readVarInt32u(in);

    // network request
    if (id == 0xC1)
    {
        RakNet::BitStream bs;

        bs.Write(std::uint8_t{0xFE});

        bs.Write(std::int8_t{12});
        bs.Write(std::uint8_t{0x8F});

        bs.Write(std::int16_t{257});
        bs.Write(std::int16_t{00});
        bs.Write(false);
        bs.Write(std::int8_t{0});
        bs.Write(float{0});
        bs.Write(std::int8_t{0});

        peer->Send((const char*)bs.GetData(), bs.GetNumberOfBytesUsed(), LOW_PRIORITY, UNRELIABLE, 0, packet->systemAddress, false);

    }

}

Server::InitFailedError::InitFailedError(const std::string& message) : std::runtime_error(message)
{
}

Server::Server(const Config& config)
{
    RakNet::SocketDescriptor socketDescriptor(config.port, "0.0.0.0");
    const RakNet::StartupResult startupResult = m_peer->Startup(config.maxUsers, &socketDescriptor, 1);

    // All clear
    if (startupResult != RakNet::RAKNET_STARTED)
    {
        throw InitFailedError(std::string(magic_enum::enum_name(startupResult)));
    }

    auto ans =
        "MCPE;CyrexMC Custom Server;898;1.21.132;0;10;12395313820729560959;Bedrock level;Survival;1;19132;19133;0;";
    RakNet::BitStream bs;
    bs.Write(ans);
    m_peer->SetOfflinePingResponse((const char*)bs.GetData(), bs.GetNumberOfBytesUsed());

    m_peer->SetMaximumIncomingConnections(config.maxIncomingConnections);
}

Server::~Server()
{
    stop();
}

Server::Server(Server&& other) noexcept : m_peer{other.m_peer}
{
    other.m_peer = nullptr;
}

Server& Server::operator=(Server&& other) noexcept
{
    stop();
    m_peer = other.m_peer;
    other.m_peer = nullptr;
    return *this;
}

void Server::run()
{
    while (true)
    {
        receivePackets();
        RakSleep(15);
    }
}

void Server::stop()
{
    if (m_peer && m_peer->IsActive())
    {
        m_peer->Shutdown(50);
    }
}

void Server::receivePackets()
{
    for (RakNet::Packet* packet{}; (packet = m_peer->Receive()) != nullptr; m_peer->DeallocatePacket(packet))
    {
        onPacketReceived(packet);
    }
}

void Server::onPacketReceived(RakNet::Packet* packet)
{
    const auto PacketIdType = static_cast<DefaultMessageIDTypes>(packet->data[0]);

    if (PacketIdType == ID_UNCONNECTED_PING)
    {
        return;
    }

    std::println("New Packet {} ({})", std::string(magic_enum::enum_name(PacketIdType)), (int)PacketIdType);

    DumpPacketHex(packet);

    switch (packet->data[0])
    {
        // G
        case 0xFE:
            handleGamePacket(m_peer, packet);
            //if(auto data = deflate::inflate({packet->data + 1, packet->length - 1}))
            {
            
            }
            break;
    }
}
