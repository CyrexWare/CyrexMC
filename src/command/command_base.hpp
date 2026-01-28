#pragma once

#include "permission.hpp"

#include <string>
#include <vector>

namespace cyrex::command
{

struct CommandContext;

class CommandBase
{
public:
    virtual ~CommandBase() = default;

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::vector<std::string> aliases() const
    {
        return {};
    }
    [[nodiscard]] virtual std::string description() const = 0;
    [[nodiscard]] virtual Permission permission() const = 0;

    virtual void execute(CommandContext& ctx) = 0;
};
} // namespace cyrex::command
