#include "server.hpp"

#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <vector>
#include <span>

// Raknet
#include "RakNet/BitStream.h"
#include "RakNet/MessageIdentifiers.h"
#include <RakNet/RakSleep.h>

#include "raknet-utils.hpp"
#include "client.hpp"


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

    RakNetUtils::dumpPacketHex(packet);

    switch (packet->data[0])
    {
        // G
        case 0xFE:
            clients[packet->guid].handleGamePacket(m_peer, packet);
            break;
    }
}
