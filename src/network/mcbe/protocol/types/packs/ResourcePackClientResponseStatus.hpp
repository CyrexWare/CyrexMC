#pragma once
#include <cstdint>

namespace cyrex::nw::proto
{

enum class ResourcePackClientResponseStatus : uint8_t
{
    Refused = 1,
    SendPacks = 2,
    HaveAllPacks = 3,
    Completed = 4
};

} // namespace cyrex::nw::proto
