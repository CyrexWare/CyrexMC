#pragma once
#include <array>
#include <bit>
#include <stdexcept>
#include <string>
#include <uuid.h>

namespace cyrex::uuid
{
using UUID = uuids::uuid;

inline std::string toString(const UUID& uuid)
{
    return uuids::to_string(uuid);
}

inline UUID fromString(const std::string& str)
{
    const auto opt = uuids::uuid::from_string(str);
    if (!opt)
        throw std::runtime_error("Invalid UUID string: " + str);
    return *opt;
}

inline UUID fromBytes(const std::array<uint8_t, 16>& bytes) noexcept
{
    return std::bit_cast<UUID>(bytes);
}

inline std::array<uint8_t, 16> toBytes(const UUID& uuid) noexcept
{
    return std::bit_cast<std::array<uint8_t, 16>>(uuid);
}

inline UUID randomUUID()
{
    static std::mt19937 engine{std::random_device{}()};
    static uuids::uuid_random_generator gen{engine};
    return gen();
}
} // namespace cyrex::uuid
