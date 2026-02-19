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

    static constexpr auto r26u00 = 924;

    static constexpr std::uint32_t currentProtocol = r26u00;

    static constexpr std::array<std::uint32_t, 2> acceptedProtocols = {898, r26u00};
};

constexpr bool isSupportedProtocol(std::uint32_t protocol)
{
    return std::ranges::any_of(ProtocolInfo::acceptedProtocols, [protocol](auto p) { return p == protocol; });
}
} // namespace cyrex::nw::protocol
