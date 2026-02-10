#pragma once

#include <string>

namespace cyrex::nw::protocol
{

struct ExperimentData
{
    std::string name;
    bool enabled = false;
};

} // namespace cyrex::nw::protocol::types::experiments
