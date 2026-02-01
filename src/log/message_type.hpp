#pragma once
#include "text/format/color.hpp"

namespace cyrex::log
{
enum class MessageType
{
    MCBE_LOG,
    MCBE_DEBUG,
    MCBE_INFO,
    MCBE_WARN,
    MCBE_ERROR,

    LOG,
    DEBUG,
    INFO,
    WARN,
    E_RROR,

    RAKNET_LOG,
    RAKNET_DEBUG,
    RAKNET_WARN,
    RAKNET_ERROR,
};

struct MessageStyle
{
    const char* tag;
    cyrex::text::format::Color color;
};

inline MessageStyle style(MessageType type)
{
    switch (type)
    {
        case MessageType::MCBE_LOG:
            return {"[Mcbe]", cyrex::text::format::Color::GREEN};
        case MessageType::MCBE_DEBUG:
            return {"[Mcbe][Debug]", cyrex::text::format::Color::GREEN};
        case MessageType::MCBE_INFO:
            return {"[Mcbe][Info]", cyrex::text::format::Color::AQUA};
        case MessageType::MCBE_WARN:
            return {"[Mcbe][Warn]", cyrex::text::format::Color::GOLD};
        case MessageType::MCBE_ERROR:
            return {"[Mcbe][Error]", cyrex::text::format::Color::RED};

        case MessageType::LOG:
            return {"[Cyrex]", cyrex::text::format::Color::BLUE};
        case MessageType::DEBUG:
            return {"[Cyrex][Debug]", cyrex::text::format::Color::BLUE};
        case MessageType::INFO:
            return {"[Cyrex][Info]", cyrex::text::format::Color::AQUA};
        case MessageType::WARN:
            return {"[Cyrex][Warn]", cyrex::text::format::Color::GOLD};
        case MessageType::E_RROR:
            return {"[Cyrex][Error]", cyrex::text::format::Color::RED};

        case MessageType::RAKNET_LOG:
            return {"[RakNet]", cyrex::text::format::Color::RED};
        case MessageType::RAKNET_DEBUG:
            return {"[RakNet][Debug]", cyrex::text::format::Color::RED};
        case MessageType::RAKNET_WARN:
            return {"[RakNet][Warn]", cyrex::text::format::Color::GOLD};
        case MessageType::RAKNET_ERROR:
            return {"[RakNet][Error]", cyrex::text::format::Color::RED};
    }

    return {"[UNKNOWN]", cyrex::text::format::Color::GRAY};
}
} // namespace cyrex::log
