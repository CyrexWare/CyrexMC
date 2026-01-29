#include "performance.hpp"

#include "text/format/builder.hpp"

#include <chrono>
#include <sstream>
#include <thread>

namespace cyrex::util
{
std::string getReport()
{
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    auto uptimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    auto uptimeMinutes = uptimeSeconds / 60;
    auto uptimeHours = uptimeMinutes / 60;
    uptimeSeconds %= 60;
    uptimeMinutes %= 60;

    cyrex::text::format::Builder b;

    b.color(cyrex::text::format::Color::AQUA)
        .style(cyrex::text::format::Style::BOLD)
        .text("=== Server Performance ===")
        .reset()
        .text("\n");

    b.color(cyrex::text::format::Color::YELLOW)
        .text("Uptime: ")
        .color(cyrex::text::format::Color::WHITE)
        .text(std::to_string(uptimeHours))
        .text("h ")
        .text(std::to_string(uptimeMinutes))
        .text("m ")
        .text(std::to_string(uptimeSeconds))
        .text("s\n");

    b.color(cyrex::text::format::Color::YELLOW)
        .text("CPU Threads: ")
        .color(cyrex::text::format::Color::WHITE)
        .text(std::to_string(std::thread::hardware_concurrency()))
        .text("\n");

    b.color(cyrex::text::format::Color::YELLOW)
        .text("Clock: ")
        .color(cyrex::text::format::Color::GRAY)
        .text("steady_clock\n");

    b.color(cyrex::text::format::Color::DARK_GRAY).text("==========================").reset();

    return b.build();
}
} // namespace cyrex::util
