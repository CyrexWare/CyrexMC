//
// Created by Jamie on 31/01/2026.
//

#pragma once
#include <functional>

namespace cyrex
{
    struct CommandArgs
    {

    };

    using Command = std::function<void(const CommandArgs&)>;
}