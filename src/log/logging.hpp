#pragma once

#include "../text/color.hpp"
#include "level.hpp"
#include "util/time.hpp"

#include <format>
#include <print>
#include <string_view>

namespace cyrex::logging
{

struct MessageCategory
{
    using GroupId = std::string_view;
    GroupId group;
    GroupId subgroup;
    AnsiColor color = AnsiColor::GRAY;
    bool enabledByDefault = true;
};

constexpr MessageCategory defaultCategory{.group = "CyrexMc"};

template <class... Args>
void print(MessageLevel level, MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    using namespace literals;

    std::string out;
    out = std::format("{}{}[{}]{}", out, AnsiColor::DARK_GRAY, util::currentTime(), "CLEAR"_ac);

    if (!category.group.empty())
    {
        out = std::format("{}{}[{}]{}", out, category.color, category.group, "CLEAR"_ac);
    }

    if (!category.subgroup.empty())
    {
        out = std::format("{}{}[{}]{}", out, category.color, category.subgroup, "CLEAR"_ac);
    }

    const std::string msg = std::format(fmt, std::forward<Args>(args)...);

    out = std::format("{} {}", out, msg);
    std::println("{}{}{}", messageLevelToAnsiColor(level), out, "CLEAR"_ac);
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

// NOLINTBEGIN
constexpr cyrex::logging::MessageCategory LOG_MCBE{.group = "MCBE", .subgroup = "", .color = cyrex::logging::AnsiColor::GREEN};
constexpr cyrex::logging::MessageCategory LOG_RAKNET{.group = "RAKNET", .subgroup = "", .color = cyrex::logging::AnsiColor::BLUE};
// NOLINTEND
