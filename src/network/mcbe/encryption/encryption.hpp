#pragma once

#include <string_view>
#include <vector>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/sha512.h>

#include <cstdint>

namespace cyrex::network::mcbe::encryption //TODO: change the name in the future
{
struct AES {
    int64_t encryptBlockCounter = 0;
    int64_t decryptBlockCounter = 0;
    ecc_key& serverKey;
    Aes& encryptBlock;
    Aes& decryptBlock;
    uint8_t* key;
    uint8_t* salt;
    bool isEncryptionEnabled;
};
bool initializeAes(const AES& aes, ecc_key& serverKey, std::string_view playerKey);
void cleanupAes(AES& aes);
bool encrypt(AES& aes, const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output);
bool decrypt(AES& aes, const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output);
} // namespace cyrex::network::mcbe::encryption