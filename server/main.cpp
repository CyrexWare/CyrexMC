#include "log/console_logger.hpp"
#include "log/message_type.hpp"
#include "network/mcbe/packet_pool.hpp"
#include "server.hpp"
#include "text/format/builder.hpp"
#include "text/format/color.hpp"
#include "util/server_properties.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <csignal>

#if defined(_WIN32)
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

namespace
{

std::filesystem::path lockFile = "server.lock";

uint64_t getPid()
{
#if defined(_WIN32)
    return static_cast<uint64_t>(GetCurrentProcessId());
#else
    return static_cast<uint64_t>(getpid());
#endif
}

bool isProcessAlive(uint64_t pid)
{
#if defined(_WIN32)
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (!h)
        return false;
    DWORD code = 0;
    const bool alive = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
    CloseHandle(h);
    return alive;
#else
    return kill(static_cast<pid_t>(pid), 0) == 0;
#endif
}

void removeLock()
{
    std::error_code ec;
    std::filesystem::remove(lockFile, ec);
}

void onExit()
{
    removeLock();
}

void onSignal(int)
{
    removeLock();
    std::_Exit(0);
}
} // namespace

int main()
{
    using namespace cyrex::util;

    if (std::filesystem::exists(lockFile))
    {
        uint64_t pid = 0;
        std::ifstream in(lockFile);
        in >> pid;

        if (pid != 0 && isProcessAlive(pid))
        {
            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::E_RROR,
                                           cyrex::text::format::Builder()
                                               .color(cyrex::text::format::Color::RED)
                                               .text("Another instance of the server is already running.\n")
                                               .build());

            cyrex::log::sendConsoleMessage(cyrex::log::MessageType::E_RROR,
                                           cyrex::text::format::Builder()
                                               .color(cyrex::text::format::Color::RED)
                                               .text("Press ENTER to safely close this instance.")
                                               .build());

            std::cin.get();
            return 1;
        }

        removeLock();
    }

    {
        std::ofstream out(lockFile, std::ios::trunc);
        out << getPid();
    }

    std::atexit(onExit);
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

#if !defined(_WIN32)
    std::signal(SIGHUP, onSignal);
#endif

    auto props = cyrex::util::ServerProperties::load("server.properties");
    cyrex::Server server(cyrex::Server::Config::fromProperties(props));
    cyrex::network::mcbe::PacketPool::instance().registerAll();
    server.run();

    removeLock();
    return 0;
}