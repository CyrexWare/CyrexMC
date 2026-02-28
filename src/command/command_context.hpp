#pragma once

#include "permission.hpp"

#include <string>
#include <vector>

namespace cyrex
{
class Server;
}

namespace cyrex::command
{

struct CommandContext
{
    Server& server;
    std::string raw;
    std::vector<std::string> args;
    PermissionLevel sourceLevel;
};
} // namespace cyrex::command
