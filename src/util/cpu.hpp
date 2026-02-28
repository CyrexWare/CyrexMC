#pragma once
#include <chrono>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <cstdint>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cyrex::util
{

class CPU
{
public:
    CPU() noexcept
    {
        init();
    }
    explicit CPU(bool auto_init) noexcept
    {
        if (auto_init)
            init();
    }

    void init() noexcept
    {
#ifdef _WIN32
        ULARGE_INTEGER uSysKernel{}, uSysUser{}, uProcKernel{}, uProcUser{};
        FILETIME sysIdleFT{}, sysKernelFT{}, sysUserFT{};
        FILETIME procCreation{}, procExit{}, procKernelFT{}, procUserFT{};
        if (GetSystemTimes(&sysIdleFT, &sysKernelFT, &sysUserFT))
        {
            uSysKernel.LowPart = sysKernelFT.dwLowDateTime;
            uSysKernel.HighPart = sysKernelFT.dwHighDateTime;
            uSysUser.LowPart = sysUserFT.dwLowDateTime;
            uSysUser.HighPart = sysUserFT.dwHighDateTime;
            prevSys_ = uSysKernel.QuadPart + uSysUser.QuadPart;
        }
        else
        {
            prevSys_ = 0;
        }
        HANDLE h = GetCurrentProcess();
        if (GetProcessTimes(h, &procCreation, &procExit, &procKernelFT, &procUserFT))
        {
            uProcKernel.LowPart = procKernelFT.dwLowDateTime;
            uProcKernel.HighPart = procKernelFT.dwHighDateTime;
            uProcUser.LowPart = procUserFT.dwLowDateTime;
            uProcUser.HighPart = procUserFT.dwHighDateTime;
            prevProc_ = uProcKernel.QuadPart + uProcUser.QuadPart;
        }
        else
        {
            prevProc_ = 0;
        }
        lastSample_ = Clock::now();
#else
        prevTotal_ = 0;
        prevProc_ = 0;
        {
            std::ifstream f("/proc/stat");
            std::string line;
            if (std::getline(f, line))
            {
                std::istringstream iss(line);
                std::string label;
                iss >> label;
                uint64_t part = 0;
                uint64_t sum = 0;
                while (iss >> part)
                    sum += part;
                prevTotal_ = sum;
            }
        }
        {
            std::ifstream f("/proc/self/stat");
            std::string line;
            if (std::getline(f, line))
            {
                auto pr = parseProcStat(line);
                prevProc_ = pr ? (*pr).first + (*pr).second : 0;
            }
        }
        lastSample_ = Clock::now();
#endif
    }

    double getUsage() noexcept
    {
#ifdef _WIN32
        ULARGE_INTEGER uSysKernel{}, uSysUser{}, uProcKernel{}, uProcUser{};
        FILETIME sysIdleFT{}, sysKernelFT{}, sysUserFT{};
        FILETIME procCreation{}, procExit, procKernelFT{}, procUserFT{};
        if (!GetSystemTimes(&sysIdleFT, &sysKernelFT, &sysUserFT))
            return 0.0;
        uSysKernel.LowPart = sysKernelFT.dwLowDateTime;
        uSysKernel.HighPart = sysKernelFT.dwHighDateTime;
        uSysUser.LowPart = sysUserFT.dwLowDateTime;
        uSysUser.HighPart = sysUserFT.dwHighDateTime;
        uint64_t nowSys = uSysKernel.QuadPart + uSysUser.QuadPart;
        HANDLE h = GetCurrentProcess();
        if (!GetProcessTimes(h, &procCreation, &procExit, &procKernelFT, &procUserFT))
            return 0.0;
        uProcKernel.LowPart = procKernelFT.dwLowDateTime;
        uProcKernel.HighPart = procKernelFT.dwHighDateTime;
        uProcUser.LowPart = procUserFT.dwLowDateTime;
        uProcUser.HighPart = procUserFT.dwHighDateTime;
        uint64_t nowProc = uProcKernel.QuadPart + uProcUser.QuadPart;
        uint64_t sysDiff = nowSys - prevSys_;
        uint64_t procDiff = nowProc - prevProc_;
        prevSys_ = nowSys;
        prevProc_ = nowProc;
        if (sysDiff == 0)
            return 0.0;
        double usage = (static_cast<long double>(procDiff) / static_cast<long double>(sysDiff)) * 100.0;
        return usage;
#else
        uint64_t total = 0;
        {
            std::ifstream f("/proc/stat");
            std::string line;
            if (!std::getline(f, line))
                return 0.0;
            std::istringstream iss(line);
            std::string label;
            iss >> label;
            uint64_t part = 0;
            while (iss >> part)
                total += part;
        }
        uint64_t proc = 0;
        {
            std::ifstream f("/proc/self/stat");
            std::string line;
            if (!std::getline(f, line))
                return 0.0;
            auto pr = parseProcStat(line);
            if (!pr)
                return 0.0;
            proc = pr->first + pr->second;
        }
        uint64_t totalDiff = (total > prevTotal_) ? (total - prevTotal_) : 0;
        uint64_t procDiff = (proc > prevProc_) ? (proc - prevProc_) : 0;
        prevTotal_ = total;
        prevProc_ = proc;
        if (totalDiff == 0)
            return 0.0;
        double usage = (static_cast<long double>(procDiff) / static_cast<long double>(totalDiff)) * 100.0;
        return usage;
#endif
    }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point lastSample_{};

#ifdef _WIN32
    uint64_t prevSys_ = 0;
    uint64_t prevProc_ = 0;
#else
    uint64_t prevTotal_ = 0;
    uint64_t prevProc_ = 0;

    static std::optional<std::pair<uint64_t, uint64_t>> parseProcStat(std::string_view line) noexcept
    {
        auto open = line.find('(');
        auto close = line.rfind(')');
        if (open == std::string_view::npos || close == std::string_view::npos || close <= open)
            return std::nullopt;
        std::string_view rest = line.substr(close + 2);
        std::istringstream iss(std::string(rest));
        std::vector<std::string> tokens;
        for (std::string tok; iss >> tok;)
            tokens.push_back(std::move(tok));
        if (tokens.size() <= 12)
            return std::nullopt;
        uint64_t utime = 0;
        uint64_t stime = 0;
        try
        {
            utime = static_cast<uint64_t>(std::stoull(tokens[11]));
            stime = static_cast<uint64_t>(std::stoull(tokens[12]));
        } catch (...)
        {
            return std::nullopt;
        }
        return std::make_pair(utime, stime);
    }
#endif
};

} // namespace cyrex::util