#pragma once

#include <cstdint>

namespace cyrex::network::protocol
{
enum class SubClientId : std::uint8_t
{
    PrimaryClient,
    Client2,
    Client3,
    Client4
};
} // namespace cyrex::network::protocol
