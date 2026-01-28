#pragma once

#include "command/command_base.hpp"

namespace cyrex::command::types
{
class StopCommand final : public CommandBase
{
public:
    std::string name() const override;
    std::vector<std::string> aliases() const override;
    std::string description() const override;
    Permission permission() const override;
    void execute(CommandContext& ctx) override;
};
} // namespace cyrex::command::types
