#include "performance.hpp"

#include "util/textformat.hpp"

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

    std::ostringstream out;

    out << bedrock(Color::AQUA) << bedrock(Style::BOLD) << "=== Server Performance ===" << bedrock(Style::RESET) << "\n";

    out << bedrock(Color::YELLOW) << "Uptime: " << bedrock(Color::WHITE) << uptimeHours << "h " << uptimeMinutes << "m "
        << uptimeSeconds << "s\n";

    out << bedrock(Color::YELLOW) << "CPU Threads: " << bedrock(Color::WHITE) << std::thread::hardware_concurrency()
        << "\n";

    out << bedrock(Color::YELLOW) << "Clock: " << bedrock(Color::GRAY) << "steady_clock\n";

    out << bedrock(Color::DARK_GRAY) << "==========================" << bedrock(Style::RESET);

    return out.str();
}
} // namespace cyrex::util
