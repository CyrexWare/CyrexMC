#pragma once
#include "server.hpp"

#include <string>

namespace cyrex::nw::raknet
{
std::string buildRaknetMotd(const cyrex::Server& server);
}
