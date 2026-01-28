#include "mcbe_batch_decoder.hpp"

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"

using namespace cyrex::network::io;

//TODO: use this again, right now it has no official purpose, in test builds, i worked on, this had a purpose
// However now no longer, this should be recoded, COMPLETELY, its faulty and dont uses session->compressor.
bool cyrex::network::mcbe::McbeBatchDecoder::decode(const uint8_t* data, size_t len, std::vector<McbeBatchPacket>& out)
{
    if (len < 1)
        return false;

    BinaryReader in(data, len);

    const uint8_t compression = in.readU8();

    // Until NetworkSettingsPacket is exchanged,
    // MCBE packets are NOT compressed
    if (compression != 0x00)
        return false;

    while (in.remaining() > 0)
    {
        const uint32_t packetLen = in.readVarInt();

        if (packetLen == 0 || packetLen > in.remaining())
            return false;

        McbeBatchPacket pkt;
        pkt.data.resize(packetLen);

        for (uint32_t i = 0; i < packetLen; ++i)
        {
            pkt.data[i] = in.readU8();
        }

        out.push_back(std::move(pkt));
    }

    return true;
}
