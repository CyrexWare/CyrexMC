#pragma once

#include "../text/color.hpp"

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

[[nodiscard]] constexpr AnsiColor messageLevelToAnsiColor(const MessageLevel level) noexcept
{
    switch (level)
    {
        case MessageLevel::Info: return AnsiColor::DARK_GRAY;
        case MessageLevel::Log: return AnsiColor::GRAY;
        case MessageLevel::Warn: return AnsiColor::YELLOW;
        case MessageLevel::Error: return AnsiColor::RED;
        case MessageLevel::Fatal: return AnsiColor::RED;
    }
}

} // namespace cyrex::logging