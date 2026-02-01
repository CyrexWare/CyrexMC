#pragma once

#include "color.hpp"

#include <format>
#include <string_view>
#include <unordered_map>

namespace cyrex::logging
{
enum class MessageLevel
{
    Info,
    Log,
    Warn,
    Error,
    Fatal,
};

inline const std::unordered_map<MessageLevel, Color>
    levelToColor{{Info, Color::DARK_GRAY}, {Log, Color::GRAY}, {Warn, Color::YELLOW}, {Error, Color::RED}, {Fatal, Color::RED}};
} // namespace cyrex::logging
