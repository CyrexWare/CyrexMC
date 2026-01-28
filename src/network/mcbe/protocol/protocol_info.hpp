#pragma once

#include <array>

#include <cstdint>

namespace cyrex::network::mcbe::protocol
{

struct ProtocolInfo
{
    static constexpr std::uint32_t CURRENT_PROTOCOL = 898;
    static constexpr const char* MINECRAFT_VERSION = "v1.21.130";
    static constexpr const char* MINECRAFT_VERSION_NETWORK = "1.21.130";

    static constexpr std::array<std::uint32_t, 1> ACCEPTED_PROTOCOLS = {CURRENT_PROTOCOL};

    static constexpr std::uint8_t LOGIN_PACKET = 0x01;
    static constexpr std::uint8_t PLAY_STATUS_PACKET = 0x02;
    static constexpr std::uint8_t SERVER_TO_CLIENT_HANDSHAKE_PACKET = 0x03;
    static constexpr std::uint8_t CLIENT_TO_SERVER_HANDSHAKE_PACKET = 0x04;
    static constexpr std::uint8_t RESOURCE_PACKS_INFO_PACKET = 0x06;
    static constexpr std::uint8_t RESOURCE_PACK_STACK_PACKET = 0x07;
    static constexpr std::uint8_t START_GAME_PACKET = 0x0b;
    static constexpr std::uint8_t LEVEL_CHUNK_PACKET = 0x3a;
    static constexpr std::uint8_t REQUEST_CHUNK_RADIUS_PACKET = 0x45;
    static constexpr std::uint8_t CHUNK_RADIUS_UPDATED_PACKET = 0x46;
    static constexpr std::uint8_t NETWORK_SETTINGS_PACKET = 0x8f;
    static constexpr std::uint8_t REQUEST_NETWORK_SETTINGS_PACKET = 0xc1;
};

} // namespace cyrex::network::mcbe::protocol
