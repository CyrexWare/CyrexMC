#pragma once
#include <string_view>

#include <cctype>
#include <cstdint>

namespace cyrex::nw::protocol
{

enum class GameMode : std::uint8_t
{
    SURVIVAL = 0,
    CREATIVE = 1,
    ADVENTURE = 2,
    SURVIVAL_VIEWER = 3,
    CREATIVE_VIEWER = 4,
    DEFAULT = 5
};

constexpr std::string_view toString(GameMode mode) noexcept
{
    switch (mode)
    {
        case GameMode::SURVIVAL:
            return "Survival";
        case GameMode::CREATIVE:
            return "Creative";
        case GameMode::ADVENTURE:
            return "Adventure";
        case GameMode::SURVIVAL_VIEWER:
            return "Survival Viewer";
        case GameMode::CREATIVE_VIEWER:
            return "Creative Viewer";
        case GameMode::DEFAULT:
            return "Default";
        default:
            return "Unknown";
    }
}

constexpr bool iequals(std::string_view a, std::string_view b) noexcept
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::toupper(static_cast<unsigned char>(a[i])) != std::toupper(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

constexpr GameMode fromString(std::string_view str) noexcept
{
    if (iequals(str, "survival") || iequals(str, "s"))
    {
        return GameMode::SURVIVAL;
    }
    if (iequals(str, "creative") || iequals(str, "c"))
    {
        return GameMode::CREATIVE;
    }
    if (iequals(str, "adventure") || iequals(str, "a"))
    {
        return GameMode::ADVENTURE;
    }
    if (iequals(str, "survival_viewer") || iequals(str, "survival viewer") || iequals(str, "spectator") ||
        iequals(str, "spec"))
    {
        return GameMode::SURVIVAL_VIEWER;
    }
    if (iequals(str, "creative_viewer") || iequals(str, "creative viewer"))
    {
        return GameMode::CREATIVE_VIEWER;
    }
    if (iequals(str, "default"))
    {
        return GameMode::DEFAULT;
    }

    return GameMode::DEFAULT;
}

} // namespace cyrex::nw::protocol
