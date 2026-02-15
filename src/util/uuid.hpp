#pragma once
#include <array>
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

inline UUID bytesToUUID(const std::array<uint8_t, 16>& bytes)
{
    UUID result;
    std::memcpy(&result, bytes.data(), 16);
    return result;
}

inline std::array<uint8_t, 16> uuidToBytes(const UUID& uuid)
{
    std::array<uint8_t, 16> out{};
    std::memcpy(out.data(), &uuid, 16);
    return out;
}
} // namespace cyrex::util
