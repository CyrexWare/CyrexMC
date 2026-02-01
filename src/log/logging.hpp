#pragma once

#include "color.hpp"
#include "level.hpp"
#include "util/time.hpp"

#include <format>
#include <print>
#include <string_view>
#include <unordered_map>

namespace cyrex::logging
{

struct MessageCategory
{
    using GroupId = std::string_view;

    GroupId group;
    GroupId subgroup;

    Color color = Color::GRAY;

    bool enabledByDefault = true;
};

constexpr MessageCategory defaultCategory{"CyrexMC", "", cyrex::logging::Color::DARK_RED};

template <class... Args>
void print(MessageLevel level, MessageCategory category, std::format_string<Args...> fmt, Args&&... args)
{
    std::string out;

    out += std::format("{}[{}]{}", Color::DARK_GRAY, cyrex::util::currentTime(), Color::CLEAR);

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

    std::println("{}{}{}", levelToColor.at(level), out, Color::CLEAR);
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
constexpr cyrex::logging::MessageCategory LOG_MCBE{"MCBE", "", cyrex::logging::Color::GREEN};
constexpr cyrex::logging::MessageCategory LOG_RAKNET{"RakNet", "", cyrex::logging::Color::BLUE};
// NOLINTEND
