#pragma once

#include <RakNet/RakNetTypes.h>

#include <cstddef>
#include <cstdint>

namespace cyrex::network::protocol
{

class Transport
{
public:
    virtual ~Transport() = default;

    virtual void send(const RakNet::RakNetGUID& guid, const uint8_t* data, size_t len) = 0;
};
} // namespace cyrex::network::protocol
