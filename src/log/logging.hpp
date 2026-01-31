#pragma once

#include "util/time.hpp"

#include <format>
#include <print>
#include <string_view>
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

inline const std::unordered_map<Color, std::string_view> colorToConsoleCode
{
    {Color::CLEAR,      "\033[0m"},
    {Color::GOLD,       "\033[33m"},
    {Color::BLACK,      "\033[30m"},
    {Color::DARK_GRAY,  "\033[90m"},
    {Color::GRAY,       "\033[37m"},
    {Color::WHITE,      "\033[97m"},
    {Color::BLUE,       "\033[94m"},
    {Color::DARK_BLUE,  "\033[34m"},
    {Color::GREEN,      "\033[92m"},
    {Color::DARK_GREEN, "\033[32m"},
    {Color::AQUA,       "\033[96m"},
    {Color::DARK_AQUA,  "\033[36m"},
    {Color::RED,        "\033[91m"},
    {Color::DARK_RED,   "\033[31m"},
    {Color::PURPLE,     "\033[95m"},
    {Color::DARK_PURPLE,"\033[35m"},
    {Color::BLUE,       "\033[94m"},
    {Color::DARK_BLUE,  "\033[34m"},
    {Color::YELLOW,     "\033[93m"},      
    {Color::DARK_YELLOW,"\033[33m"}
};

enum class MessageLevel
{
    Info,
    Log,
    Warn,
    Error,
    Fatal,
};

struct MessageCategory
{
    using GroupId = std::string_view;

    GroupId group;
    GroupId subgroup;

    Color color = Color::GRAY;

    bool enabledByDefault = true;
};

constexpr MessageCategory defaultCategory{
    "CyrexMc",
    "",
};

template <class... Args>
void print(MessageLevel level, MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    std::string out;

    if (true) // includeTime
    {
        out += std::format("{}[{}]{}", Color::DARK_GRAY, cyrex::util::currentTime(), Color::CLEAR);
    }

    if (!category.group.empty())
    {
        out += std::format("{}[{}]{}", category.color, category.group, Color::CLEAR);
    }

    if (!category.subgroup.empty())
    {
        out += std::format("{}[{}]{}", category.color, category.subgroup, Color::CLEAR);
    }

    out += " ";

    out += std::format(fmt, std::forward<Args>(args)...);

    std::println("{}{}", out, Color::CLEAR);
}

template <class... Args>
void info(MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Info, category, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void log(MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Log, category, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void warn(MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Warn, category, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void error(MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Error, category, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void fatal(MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Fatal, category, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void info(std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Info, defaultCategory, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void log(std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Log, defaultCategory, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void warn(std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Warn, defaultCategory, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void error(std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Error, defaultCategory, fmt, std::forward<Args>(args)...);
}

template <class... Args>
void fatal(std::format_string<Args...> fmt, Args&&... args)
{
    print(MessageLevel::Fatal, defaultCategory, fmt, std::forward<Args>(args)...);
}

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

constexpr cyrex::logging::MessageCategory LOG_MCBE{"MCBE", "", cyrex::logging::Color::GREEN};

constexpr cyrex::logging::MessageCategory LOG_RAKNET{"RAKNET", "", cyrex::logging::Color::BLUE};
