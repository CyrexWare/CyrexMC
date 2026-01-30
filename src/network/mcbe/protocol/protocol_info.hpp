#pragma once

#include <algorithm>
#include <array>

#include <cstdint>

namespace cyrex::network::mcbe::protocol
{

struct ProtocolInfo
{
    static constexpr std::uint32_t currentProtocol = 898;
    static constexpr const char* minecraftVersion = "v1.21.130";
    static constexpr const char* minecraftVersionNetwork = "1.21.130";

    static constexpr std::array<std::uint32_t, 1> acceptedProtocols = {currentProtocol};

    static constexpr std::uint8_t loginPacket = 0x01;
    static constexpr std::uint8_t playStatusPacket = 0x02;
    static constexpr std::uint8_t serverToClientHandshakePacket = 0x03;
    static constexpr std::uint8_t clientToServerHandshakePacket = 0x04;
    static constexpr std::uint8_t resourcePacksInfoPacket = 0x06;
    static constexpr std::uint8_t resourcePackStackPacket = 0x07;
    static constexpr std::uint8_t startGamePacket = 0x0b;
    static constexpr std::uint8_t levelChunkPacket = 0x3a;
    static constexpr std::uint8_t requestChunkRadiusPacket = 0x45;
    static constexpr std::uint8_t chunkRadiusUpdatedPacket = 0x46;
    static constexpr std::uint8_t networkSettingsPacket = 0x8f;
    static constexpr std::uint8_t requestNetworkSettingsPacket = 0xc1;
};


// no way im planning multiprotocol??
constexpr bool isProtocolMabyeAccepted(std::uint32_t protocol)
{
    return std::any_of(ProtocolInfo::acceptedProtocols.begin(),
                       ProtocolInfo::acceptedProtocols.end(),
                       [protocol](auto p) { return p == protocol; });
}
} // namespace cyrex::network::mcbe::protocol
