#pragma once

#include <format>
#include <magic_enum/magic_enum.hpp>
#include <string_view>

namespace cyrex::logging
{

enum class AnsiColor
{
    CLEAR,
    GOLD,
    BLACK,
    DARK_GRAY,
    GRAY,
    WHITE,
    BLUE,
    DARK_BLUE,
    GREEN,
    DARK_GREEN,
    AQUA,
    DARK_AQUA,
    RED,
    DARK_RED,
    PURPLE,
    DARK_PURPLE,
    YELLOW,
};

inline bool ansiColorsDisabled = false;

using AnsiEscapeSequence = std::string_view;

[[nodiscard]] constexpr AnsiEscapeSequence ansiColorToSequence(const AnsiColor color) noexcept
{
    if (ansiColorsDisabled)
    {
        return "";
    }

    switch (color)
    {
        case AnsiColor::CLEAR: return "\033[0m";
        case AnsiColor::GOLD: return "\033[33m";
        case AnsiColor::BLACK: return "\033[30m";
        case AnsiColor::DARK_GRAY: return "\033[90m";
        case AnsiColor::GRAY: return "\033[37m";
        case AnsiColor::WHITE: return "\033[97m";
        case AnsiColor::BLUE: return "\033[94m";
        case AnsiColor::DARK_BLUE: return "\033[34m";
        case AnsiColor::GREEN: return "\033[92m";
        case AnsiColor::DARK_GREEN: return "\033[32m";
        case AnsiColor::AQUA: return "\033[96m";
        case AnsiColor::DARK_AQUA: return "\033[36m";
        case AnsiColor::RED: return "\033[91m";
        case AnsiColor::DARK_RED: return "\033[31m";
        case AnsiColor::PURPLE: return "\033[95m";
        case AnsiColor::DARK_PURPLE: return "\033[35m";
        case AnsiColor::YELLOW: return "\033[93m";
    }

    std::unreachable();
}

} // namespace cyrex::logging

// user-defined literals should go in a namespace
namespace cyrex::logging::literals
{

consteval AnsiColor operator""_ac(const char* str, std::size_t len)
{
    if (len == 0 || len >= 32)
        std::unreachable(); // too long → compile-time error

    char nameBuffer[32]{};
    for (std::size_t i = 0; i < len; ++i)
        nameBuffer[i] = str[i]; // keep original case for magic_enum

    std::string_view sv{nameBuffer, len};

    if (auto maybe_val = magic_enum::enum_cast<AnsiColor>(sv); maybe_val)
        return *maybe_val;

    std::unreachable();
}

} // namespace cyrex::logging::literals


namespace std
{
template <>
struct formatter<cyrex::logging::AnsiColor> : formatter<string_view>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return formatter<string_view>::parse(ctx);
    }

    auto format(const cyrex::logging::AnsiColor color, format_context& ctx) const
    {
        return formatter<string_view>::format(cyrex::logging::ansiColorToSequence(color), ctx);
    }
};

} // namespace std