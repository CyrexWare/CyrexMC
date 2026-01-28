#pragma once

#include "command/command_base.hpp"

namespace cyrex::command::types
{
class StopCommand final : public CommandBase
{
public:
    [[nodiscard]] [[nodiscard]] std::string name() const override;
    [[nodiscard]] [[nodiscard]] std::vector<std::string> aliases() const override;
    [[nodiscard]] [[nodiscard]] std::string description() const override;
    [[nodiscard]] [[nodiscard]] Permission permission() const override;
    void execute(CommandContext& ctx) override;
};
} // namespace cyrex::command::types
