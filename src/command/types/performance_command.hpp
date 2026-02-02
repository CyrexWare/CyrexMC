#pragma once

#include "command/command_base.hpp"

namespace cyrex::command::types
{
class PerformanceCommand final : public CommandBase
{
public:
    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string description() const override;
    [[nodiscard]] Permission permission() const override;
    void execute(CommandContext& ctx) override;
};
} // namespace cyrex::command::types
