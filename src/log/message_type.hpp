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
    text::Color color;
};

inline MessageStyle style(MessageType type)
{
    using namespace text;

    switch (type)
    {
        case MessageType::MCBE_LOG:
            return {"[MCBE]", Color::GREEN};
        case MessageType::MCBE_DEBUG:
            return {"[MCBE][DEBUG]", Color::GREEN};
        case MessageType::MCBE_INFO:
            return {"[MCBE][INFO]", Color::AQUA};
        case MessageType::MCBE_WARN:
            return {"[MCBE][WARN]", Color::YELLOW};
        case MessageType::MCBE_ERROR:
            return {"[MCBE][ERROR]", Color::RED};

        case MessageType::LOG:
            return {"[Cyrex]", Color::BLUE};
        case MessageType::DEBUG:
            return {"[Cyrex][DEBUG]", Color::BLUE};
        case MessageType::INFO:
            return {"[Cyrex][INFO]", Color::AQUA};
        case MessageType::WARN:
            return {"[Cyrex][WARN]", Color::YELLOW};
        case MessageType::E_RROR:
            return {"[Cyrex][ERROR]", Color::RED};

        case MessageType::RAKNET_LOG:
            return {"[RAKNET]", Color::RED};
        case MessageType::RAKNET_DEBUG:
            return {"[RAKNET][DEBUG]", Color::RED};
        case MessageType::RAKNET_WARN:
            return {"[RAKNET][WARN]", Color::YELLOW};
        case MessageType::RAKNET_ERROR:
            return {"[RAKNET][ERROR]", Color::RED};
    }

    return {"[UNKNOWN]", Color::GRAY};
}
} // namespace cyrex::log
