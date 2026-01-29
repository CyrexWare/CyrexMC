#pragma once
#include "message_format.hpp"
#include "text/render/ansi.hpp"

#include <iostream>

namespace cyrex::log
{
inline void sendConsoleMessage(MessageType type, std::string_view message)
{
    std::cout << text::render::ansi(formatMessage(type, message)) << '\n';
}
} // namespace cyrex::log
