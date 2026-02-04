#pragma once

#include <string_view>
#include <vector>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/random.h>

#include <cstdint>

namespace cyrex::network::mcbe::encryption
{

struct AES
{
    std::int64_t encryptBlockCounter = 0;
    std::int64_t decryptBlockCounter = 0;

    ecc_key* serverKey{};
    Aes* encryptBlock{};
    Aes* decryptBlock{};

    std::vector<std::uint8_t> key;
    std::vector<std::uint8_t> salt;

    AES() = default;
    AES(ecc_key* serverKey, Aes* enc, Aes* dec) : serverKey(serverKey), encryptBlock(enc), decryptBlock(dec)
    {
    }
};

[[nodiscard]] bool initializeAes(AES aes, ecc_key& serverKey, std::string_view playerKey);
void cleanupAes(AES aes);
[[nodiscard]] bool encrypt(AES aes, const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output);
[[nodiscard]] bool decrypt(AES aes, const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output);

} // namespace cyrex::network::mcbe::encryption
