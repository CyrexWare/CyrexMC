#pragma once

#include <RakNet/RakNetTypes.h>
#include <functional>

// does this need a namespace? -- probably not
namespace std
{
template <>
struct hash<RakNet::RakNetGUID>
{
    size_t operator()(const RakNet::RakNetGUID& guid) const noexcept
    {
        return std::hash<uint64_t>{}(guid.g);
    }
};
} // namespace std
