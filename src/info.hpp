#pragma once

#include <string>

namespace cyrex
{

class Info final
{
public:
    enum class BuildType
    {
        Development,
        Preview,
        Production
    };

    struct Version
    {
        int major{};
        int minor{};
        int patch{};

        [[nodiscard]] std::string toString() const;
    };

    [[nodiscard]] static const std::string& name() noexcept;
    [[nodiscard]] static const std::string& description() noexcept;
    [[nodiscard]] static Version version() noexcept;
    [[nodiscard]] static BuildType buildType() noexcept;
    [[nodiscard]] static std::string buildTypeString();
};

} // namespace cyrex
