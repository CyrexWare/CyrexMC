#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace cyrex::util
{
inline std::string currentTime()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto tt = system_clock::to_time_t(now);

    std::tm tm{};
// according to microsoft edge (.. yes i said what i said microsoft edge)
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
}
} // namespace cyrex::util
