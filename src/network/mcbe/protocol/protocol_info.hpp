#pragma once

#include <algorithm>
#include <array>

#include <cstdint>

namespace cyrex::nw::protocol
{

struct ProtocolInfo
{
    static constexpr auto minecraftVersion = "v26.0";
    static constexpr auto minecraftVersionNetwork = "26.0";

    static constexpr auto R26U00 = 924;
    static constexpr auto R21U13 = 898;

    static constexpr std::uint32_t currentProtocol = R26U00;

    static constexpr std::array<std::uint32_t, 2> acceptedProtocols = {
        R26U00,
        R21U13
    };
};

constexpr bool isSupportedProtocol(std::uint32_t protocol)
{
    return std::ranges::any_of(ProtocolInfo::acceptedProtocols, [protocol](auto p) { return p == protocol; });
}
} // namespace cyrex::nw::protocol
