#pragma once

#include <format>
#include <unordered_map>

namespace cyrex::logging
{
enum class Color
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
    DARK_YELLOW,
};

inline const std::unordered_map<Color, std::string_view>
    colorToConsoleCode{{Color::CLEAR, "\033[0m"},   {Color::GOLD, "\033[33m"},
                       {Color::BLACK, "\033[30m"},  {Color::DARK_GRAY, "\033[90m"},
                       {Color::GRAY, "\033[37m"},   {Color::WHITE, "\033[97m"},
                       {Color::BLUE, "\033[94m"},   {Color::DARK_BLUE, "\033[34m"},
                       {Color::GREEN, "\033[92m"},  {Color::DARK_GREEN, "\033[32m"},
                       {Color::AQUA, "\033[96m"},   {Color::DARK_AQUA, "\033[36m"},
                       {Color::RED, "\033[91m"},    {Color::DARK_RED, "\033[31m"},
                       {Color::PURPLE, "\033[95m"}, {Color::DARK_PURPLE, "\033[35m"},
                       {Color::BLUE, "\033[94m"},   {Color::DARK_BLUE, "\033[34m"},
                       {Color::YELLOW, "\033[93m"}, {Color::DARK_YELLOW, "\033[33m"}};
} // namespace cyrex::logging

namespace std
{
template <>
struct formatter<cyrex::logging::Color> : formatter<string_view>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return formatter<string_view>::parse(ctx);
    }

    auto format(cyrex::logging::Color color, format_context& ctx) const
    {
        return formatter<string_view>::format(cyrex::logging::colorToConsoleCode.at(color), ctx);
    }
};
} // namespace std