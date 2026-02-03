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
    using namespace cyrex::logging::literals;
    auto now = std::chrono::steady_clock::now();

    auto uptimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    auto uptimeMinutes = uptimeSeconds / 60;
    auto uptimeHours = uptimeMinutes / 60;
    uptimeSeconds %= 60;
    uptimeMinutes %= 60;

    logging::log(LOG_MCBE, "=== Server Performance ===");
    logging::log(LOG_MCBE, "Uptime: {}{}h {}m {}s", "White"_ac, uptimeHours, uptimeMinutes, uptimeSeconds);
    logging::log(LOG_MCBE, "CPU Threads: {}{}", "White"_ac, std::thread::hardware_concurrency());
    logging::log(LOG_MCBE, "==========================");
}
} // namespace cyrex::util
