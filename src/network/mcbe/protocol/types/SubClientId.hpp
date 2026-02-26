#pragma once

#include <cstdint>

namespace cyrex::nw::protocol
{
enum class SubClientId : std::uint8_t
{
    PrimaryClient,
    Client2,
    Client3,
    Client4
};
} // namespace cyrex::nw::protocol
