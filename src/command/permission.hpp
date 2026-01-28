#pragma once

#include <string>

namespace cyrex::command {

enum class PermissionLevel
{
    // more required as future goes on..
    PLAYER = 0,
    OP = 1,
    CONSOLE = 2
};

struct Permission
{
    std::string node;
    PermissionLevel level;
};
} // namespace cyrex::command