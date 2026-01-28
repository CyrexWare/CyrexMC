#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cyrex::util
{
// should make sure this is proper
constexpr char CODE = '\xA7';

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
    WHITE = 'f',

    MINECOIN_GOLD = 'g',
    QUARTZ = 'h',
    IRON = 'i',
    NETHERITE = 'j',
    REDSTONE = 'm',
    COPPER = 'n',
    GOLD_MATERIAL = 'p',
    EMERALD = 'q',
    DIAMOND = 's',
    LAPIS = 't',
    AMETHYST = 'u',
    RESIN = 'v'
};

enum class Style : char
{
    OBFUSCATED = 'k',
    BOLD = 'l',
    ITALIC = 'o',
    RESET = 'r'
};

inline std::string bedrock(Color c)
{
    return std::string{CODE, static_cast<char>(c)};
}

inline std::string bedrock(Style s)
{
    return std::string{CODE, static_cast<char>(s)};
}

inline std::string color(Color c, std::string_view text)
{
    return bedrock(c) + std::string(text) + bedrock(Style::RESET);
}

inline std::string style(Style s, std::string_view text)
{
    return bedrock(s) + std::string(text) + bedrock(Style::RESET);
}

inline std::string strip(std::string_view text)
{
    std::string out;
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == CODE && i + 1 < text.size())
        {
            ++i;
            continue;
        }
        out.push_back(text[i]);
    }
    return out;
}

// Put this instead in an utility like time, temporary
inline std::string currentTime()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto tt = system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
}


inline std::string renderConsole(std::string_view text, bool includeTime)
{
    static const std::unordered_map<char, std::string>
        ansi{{'0', "\033[30m"},
             {'1', "\033[34m"},
             {'2', "\033[32m"},
             {'3', "\033[36m"},
             {'4', "\033[31m"},
             {'5', "\033[35m"},
             {'6', "\033[33m"},
             {'7', "\033[37m"},
             {'8', "\033[90m"},
             {'9', "\033[94m"},
             {'a', "\033[92m"},
             {'b', "\033[96m"},
             {'c', "\033[91m"},
             {'d', "\033[95m"},
             {'e', "\033[93m"},
             {'f', "\033[97m"},
             {'l', "\033[1m"},
             {'o', "\033[3m"},
             {'r', "\033[0m"}};

    std::string out;

    if (includeTime)
    {
        out += "\033[90m[";
        out += currentTime();
        out += "] \033[0m";
    }

    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == CODE && i + 1 < text.size())
        {
            auto it = ansi.find(text[i + 1]);
            if (it != ansi.end())
                out += it->second;
            ++i;
            continue;
        }
        out.push_back(text[i]);
    }

    out += "\033[0m";
    return out;
}
} // namespace cyrex::util
