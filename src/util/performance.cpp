#include "performance.hpp"

#include "log/logging.hpp"

#include <chrono>
#include <sstream>
#include <thread>

namespace cyrex::util
{
const auto startTime = std::chrono::steady_clock::now();
void logServerStatus()
{
    auto now = std::chrono::steady_clock::now();

    auto uptimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    auto uptimeMinutes = uptimeSeconds / 60;
    auto uptimeHours = uptimeMinutes / 60;
    uptimeSeconds %= 60;
    uptimeMinutes %= 60;

    logging::log("=== Server Performance ===");
    logging::log("Uptime: {}{}h {}m {}s", logging::Color::WHITE, uptimeHours, uptimeMinutes, uptimeSeconds);
    logging::log("CPU Threads: {}{}", logging::Color::WHITE, std::thread::hardware_concurrency());
    logging::log("==========================");
}
} // namespace cyrex::util
