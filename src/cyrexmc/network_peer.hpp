#pragma once
#include <cstdint>
#include <string_view>
#include <RakNet/RakPeerInterface.h>

namespace cyrex
{

// Basic I/O wrapper interface for networked connections
// For making testing easier
class INetworkPeer
{
public:
    using StartupResult = RakNet::StartupResult;

    using Packet = RakNet::Packet;

    struct StartupInfo
    {
        std::uint32_t maxConnections{};
        RakNet::SocketDescriptor* socketDescriptors{};
        std::uint32_t numDescriptors{};
    };
    
    virtual StartupResult startup(const StartupInfo startupInfo) = 0;

    struct ShutdownInfo
    {
        std::uint32_t blockDuration{};
        std::uint8_t orderingChannel{};
        PacketPriority disconnectionNotificationPriority = LOW_PRIORITY;
    };

    virtual void shutdown(const ShutdownInfo shutdownInfo) = 0;

    virtual void setMaximumIncomingConnections(const std::uint16_t maxIncomingConnections) = 0;

    virtual Packet* receive() = 0;

    struct SendInfo
    {
        // Priority level to send on.  See RakNet's PacketPriority.h
        PacketPriority priority = MEDIUM_PRIORITY;
        // How reliably to send this data.  See RakNet's PacketPriority.h
        PacketReliability reliability = RELIABLE;
        // When using ordered or sequenced messages, the channel to order these on. Messages are only ordered relative to other messages on the same stream.
        char orderingChannel{};
        //  Who to send this packet to, or in the case of broadcasting who not to send it to. Pass either a SystemAddress structure or a RakNetGUID structure. Use UNASSIGNED_SYSTEM_ADDRESS or to specify none
        RakNet::AddressOrGUID systemIdentifier{RakNet::UNASSIGNED_SYSTEM_ADDRESS};
        // True to send this packet to all connected systems. If true, then systemAddress specifies who not to send the packet to.
        bool broadcast{false};
        // If 0, will automatically determine the receipt number to return. If non-zero, will return what you give it.
        uint32_t forceReceiptNumber{};
    };

    virtual std::uint32_t send(const char* data, const int length, const SendInfo sendInfo) = 0;

    virtual void deallocatePacket(Packet* packet) = 0;

    virtual bool isActive() const = 0;
};

} // namespace cyrex