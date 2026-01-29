#pragma once
#include <string>
#include <string_view>

namespace cyrex::text::format
{
constexpr char Code = '\xA7';

enum class Color : char
{
    BLACK = '0',
    DARK_BLUE = '1',
    DARK_GREEN = '2',
    DARK_AQUA = '3',
    DARK_RED = '4',
    DARK_PURPLE = '5',
    GOLD = '6',
    GRAY = '7',
    DARK_GRAY = '8',
    BLUE = '9',
    GREEN = 'a',
    AQUA = 'b',
    RED = 'c',
    LIGHT_PURPLE = 'd',
    YELLOW = 'e',
    WHITE = 'f'
};

enum class Style : char
{
    OBFUSCATED = 'k',
    BOLD = 'l',
    ITALIC = 'o',
    RESET = 'r'
};

inline std::string code(Color c)
{
    return {Code, static_cast<char>(c)};
}

inline std::string code(Style s)
{
    return {Code, static_cast<char>(s)};
}

inline std::string color(Color c, std::string_view text)
{
    return code(c) + std::string(text) + code(Style::RESET);
}
} // namespace cyrex::text::format
#pragma once
#include <string>
#include <string_view>

namespace cyrex::text
{
constexpr char Code = '\xA7';

enum class Color : char
{
    BLACK = '0',
    DARK_BLUE = '1',
    DARK_GREEN = '2',
    DARK_AQUA = '3',
    DARK_RED = '4',
    DARK_PURPLE = '5',
    GOLD = '6',
    GRAY = '7',
    DARK_GRAY = '8',
    BLUE = '9',
    GREEN = 'a',
    AQUA = 'b',
    RED = 'c',
    LIGHT_PURPLE = 'd',
    YELLOW = 'e',
    WHITE = 'f'
};

enum class Style : char
{
    OBFUSCATED = 'k',
    BOLD = 'l',
    ITALIC = 'o',
    RESET = 'r'
};

inline std::string code(Color c)
{
    return {Code, static_cast<char>(c)};
}

inline std::string code(Style s)
{
    return {Code, static_cast<char>(s)};
}

inline std::string color(Color c, std::string_view text)
{
    return code(c) + std::string(text) + code(Style::RESET);
}
} // namespace cyrex::text
