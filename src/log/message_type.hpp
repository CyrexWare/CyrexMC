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
            return {"[MCBE]", cyrex::text::format::Color::GREEN};
        case MessageType::MCBE_DEBUG:
            return {"[MCBE][DEBUG]", cyrex::text::format::Color::GREEN};
        case MessageType::MCBE_INFO:
            return {"[MCBE][INFO]", cyrex::text::format::Color::AQUA};
        case MessageType::MCBE_WARN:
            return {"[MCBE][WARN]", cyrex::text::format::Color::YELLOW};
        case MessageType::MCBE_ERROR:
            return {"[MCBE][ERROR]", cyrex::text::format::Color::RED};

        case MessageType::LOG:
            return {"[Cyrex]", cyrex::text::format::Color::BLUE};
        case MessageType::DEBUG:
            return {"[Cyrex][DEBUG]", cyrex::text::format::Color::BLUE};
        case MessageType::INFO:
            return {"[Cyrex][INFO]", cyrex::text::format::Color::AQUA};
        case MessageType::WARN:
            return {"[Cyrex][WARN]", cyrex::text::format::Color::YELLOW};
        case MessageType::E_RROR:
            return {"[Cyrex][ERROR]", cyrex::text::format::Color::RED};

        case MessageType::RAKNET_LOG:
            return {"[RAKNET]", cyrex::text::format::Color::RED};
        case MessageType::RAKNET_DEBUG:
            return {"[RAKNET][DEBUG]", cyrex::text::format::Color::RED};
        case MessageType::RAKNET_WARN:
            return {"[RAKNET][WARN]", cyrex::text::format::Color::YELLOW};
        case MessageType::RAKNET_ERROR:
            return {"[RAKNET][ERROR]", cyrex::text::format::Color::RED};
    }

    return {"[UNKNOWN]", cyrex::text::format::Color::GRAY};
}
} // namespace cyrex::log
