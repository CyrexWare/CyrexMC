#pragma once
#include <cstdint>

namespace cyrex::network::protocol
{

enum class ResourcePackDataType : uint8_t
{
    Invalid = 0,
    Addon = 1,
    Cached = 2,
    CopyProtected = 3,
    Behavior = 4,
    PersonaPiece = 5,
    Resource = 6,
    Skins = 7,
    WorldTemplate = 8,
    Count = 9
};

} // namespace cyrex::network::protocol
