#pragma once
#include "server.hpp"

#include <string>

namespace cyrex::nw::raknet
{
std::string buildMotd(const cyrex::Server& server);
}
