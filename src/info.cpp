#include "info.hpp"

#include <string>

namespace
{
const std::string kName = "CyrexMC";
const std::string
    kDescription = "C++ Minecraft Bedrock Server Software built from scratch emphasizing speed and correctness";

const cyrex::Info::Version kVersion{0, 0, 1};
const cyrex::Info::BuildType kBuildType = cyrex::Info::BuildType::Development;
} // namespace

namespace cyrex
{

[[nodiscard]] const std::string& Info::name() noexcept
{
    return kName;
}

[[nodiscard]] const std::string& Info::description() noexcept
{
    return kDescription;
}

[[nodiscard]] Info::Version Info::version() noexcept
{
    return kVersion;
}

[[nodiscard]] Info::BuildType Info::buildType() noexcept
{
    return kBuildType;
}

[[nodiscard]] std::string Info::buildTypeString()
{
    switch (buildType())
    {
        case BuildType::Development:
            return "Development";
        case BuildType::Preview:
            return "Preview";
        case BuildType::Production:
            return "Production";
    }

    return "Unknown";
}

[[nodiscard]] std::string Info::Version::toString() const
{
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

} // namespace cyrex
