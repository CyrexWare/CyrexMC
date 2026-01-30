#pragma once
#include "message_type.hpp"
#include "text/format/color.hpp"
#include "util/time.hpp"

#include <string>
#include <string_view>

namespace cyrex::log
{
inline std::string formatMessage(MessageType type, std::string_view message, bool includeTime = true)
{
    using namespace text;

    const auto meta = style(type);

    std::string out;

    if (includeTime)
    {
        out += text::format::color(text::format::Color::DARK_GRAY, "[" + cyrex::util::currentTime() + "]");
    }

    out += color(meta.color, meta.tag);
    if (!message.empty())
    {
        out += " ";
        out += std::string(message);
    }
    return out;
}
} // namespace cyrex::log
