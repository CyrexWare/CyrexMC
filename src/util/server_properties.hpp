#pragma once

#include "network/mcbe/protocol/types/GameMode.hpp"

#include <string>

#include <cstdint>

namespace cyrex::util
{
namespace proto = nw::protocol;

struct ServerProperties
{
    std::uint16_t port{};
    std::uint16_t portIpv6{};
    std::uint32_t maxPlayers{};
    std::string serverName;
    std::string motd;
    proto::GameMode defaultGameMode{};

    static ServerProperties load(const std::string& path);
};

} // namespace cyrex::util
