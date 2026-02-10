#pragma once

#include <algorithm>
#include <array>

#include <cstdint>

namespace cyrex::nw::protocol
{

struct ProtocolInfo
{
    static constexpr std::uint32_t currentProtocol = 898;
    static constexpr auto minecraftVersion = "v1.21.130";
    static constexpr auto minecraftVersionNetwork = "1.21.130";

    static constexpr std::array<std::uint32_t, 1> acceptedProtocols = {currentProtocol};
};

constexpr bool isSupportedProtocol(std::uint32_t protocol)
{
    return std::ranges::any_of(ProtocolInfo::acceptedProtocols, [protocol](auto p) { return p == protocol; });
}
} // namespace cyrex::nw::protocol
