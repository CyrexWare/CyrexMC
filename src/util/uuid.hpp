#pragma once
#include <array>
#include <bit>
#include <stdexcept>
#include <string>
#include <uuid.h>

namespace cyrex::util
{
using UUID = uuids::uuid;

inline std::string uuidToString(const UUID& uuid)
{
    return uuids::to_string(uuid);
}

inline UUID stringToUUID(const std::string& str)
{
    auto opt = uuids::uuid::from_string(str);
    if (!opt)
        throw std::runtime_error("Invalid UUID string: " + str);
    return *opt;
}

inline UUID bytesToUUID(const std::array<uint8_t, 16>& bytes) noexcept
{
    return std::bit_cast<UUID>(bytes);
}

inline std::array<uint8_t, 16> uuidToBytes(const UUID& uuid) noexcept
{
    return std::bit_cast<std::array<uint8_t, 16>>(uuid);
}
} // namespace cyrex::util
