//
// Created by Jamie on 29/01/2026.
//

#pragma once
#include <format>
#include <string_view>
#include <utility>

namespace cyrex
{
    enum class TextStyle
    {
        Black,
        DarkBlue,
        DarkGreen,
        DarkCyan,
        DarkRed,
        DarkPurple,
        DarkYellow,
        DarkGray,
        LightGray,
        LightBlue,
        LightGreen,
        LightCyan,
        LightRed,
        LightPurple,
        LightYellow,
        White,
        Obfuscated,
        Bold,
        Strikethrough,
        Underlined,
        Italic,
        Reset
    };

    [[nodiscard]] constexpr std::string_view textStyleToString(const TextStyle style) noexcept
    {
        using enum TextStyle;
        switch (style)
        {
            // Colors
            case Black: return "§0";
            case White: return "§f";
            case DarkBlue: return "§1";
            case DarkGreen: return "§2";
            case DarkCyan: return "§3";
            case DarkRed: return "§4";
            case DarkPurple: return "§5";
            case DarkYellow: return "§6";
            case LightGray: return "§7";
            case DarkGray: return "§8";
            case LightBlue: return "§9";
            case LightGreen: return "§9";
            case LightCyan: return "§9";
            case LightRed: return "§9";
            case LightPurple: return "§9";
            case LightYellow: return "§9";

            case Obfuscated: return "§k";
            case Bold: return "§l";
            case Strikethrough: return "§m";
            case Underlined: return "§n";
            case Italic: return "§o";
            case Reset: return "§r";
        }

        std::unreachable();
    }
}

template<>
struct std::formatter<cyrex::TextStyle>
{
    constexpr static auto parse(std::format_parse_context& context) {
        return ctx.begin();
    }

    // Format the value
    template <typename FormatContext>
    auto format(const cyrex::TextStyle textStyle, FormatContext& context) const {
        return std::format_to(
            context.out(),
            "{}",
            cyrex::textStyleToString(textStyle)
        );
    }

};
