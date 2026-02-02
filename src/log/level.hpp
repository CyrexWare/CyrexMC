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

inline const std::unordered_map<MessageLevel, Color> levelToColor{{MessageLevel::Info, Color::DARK_GRAY},
                                                                  {MessageLevel::Log, Color::GRAY},
                                                                  {MessageLevel::Warn, Color::GOLD},
                                                                  {MessageLevel::Error, Color::RED},
                                                                  {MessageLevel::Fatal, Color::RED}};
} // namespace cyrex::logging
