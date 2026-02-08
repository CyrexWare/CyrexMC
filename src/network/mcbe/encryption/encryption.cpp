#include "encryption.hpp"

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "wolfssl/openssl/ssl.h"

namespace cyrex::nw::protocol
{

static uint64_t calculateChecksum(const int64_t blockCounter,
                                  const std::uint8_t* key,
                                  const std::uint8_t* inputBuffer,
                                  const word32 inputSize)
{
    uint8_t hash[32];
    wc_Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, reinterpret_cast<const std::uint8_t*>(&blockCounter), 8);
    wc_Sha256Update(&sha, inputBuffer, inputSize);
    wc_Sha256Update(&sha, key, 32);
    wc_Sha256Final(&sha, hash);
    io::BinaryReader ri(hash, 32);
    return ri.readU64LE();
}

using Hash = std::uint64_t;
constexpr auto hashSize = sizeof(Hash);

std::optional<std::vector<uint8_t>> AesEncryptor::encrypt(const std::span<const uint8_t> input) const
{
    const Hash hash = calculateChecksum(encryptBlock->counter++, key.data(), input.data(), input.size());
    std::vector<std::uint8_t> buffer;
    buffer.reserve(input.size() + hashSize);
    buffer.insert(buffer.end(), input.begin(), input.end());
    buffer.insert(buffer.end(),
                  reinterpret_cast<const std::uint8_t*>(&hash),
                  reinterpret_cast<const std::uint8_t*>(&hash) + hashSize);
    std::vector<std::uint8_t> output(buffer.size());
    wc_AesCtrEncrypt(encryptBlock.get(), output.data(), buffer.data(), buffer.size());
    return output;
}

std::optional<std::vector<uint8_t>> AesEncryptor::decrypt(const std::span<const uint8_t> input) const
{
    std::vector<std::uint8_t> output(input.size());
    wc_AesCtrEncrypt(decryptBlock.get(), output.data(), input.data(), input.size());
    io::BinaryReader ri(output.data() + input.size() - hashSize, hashSize);
    const Hash receivedHash = ri.readU64LE();
    const Hash calculatedHash = calculateChecksum(decryptBlock->counter++,
                                                  key.data(),
                                                  output.data(),
                                                  static_cast<word32>(input.size() - hashSize));
    if (calculatedHash != receivedHash)
    {
        return std::nullopt;
    }

    output.resize(input.size() - hashSize);
    return output;
}

} // namespace cyrex::nw::protocol
