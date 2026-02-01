#include "encryption.hpp"

#include "network/io/binary_reader.hpp"
#include "network/io/binary_writer.hpp"
#include "wolfssl/openssl/ssl.h"

namespace cyrex::network::mcbe::encryption
{
static uint64_t calculateChecksum(const int64_t blockCounter, const uint8_t* key, const uint8_t *inputBuffer, const word32 inputSize)
{
    uint8_t hash[32];
    io::BinaryWriter bi{};
    bi.writeU64LE(blockCounter);
    wc_Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, bi.data(), 8);
    wc_Sha256Update(&sha, inputBuffer, inputSize);
    wc_Sha256Update(&sha, key, 32);
    wc_Sha256Final(&sha, hash);
    io::BinaryReader ri(hash, 32);
    return ri.readU64LE();
}

bool initializeAes(AES& aes, ecc_key& serverKey, std::string_view playerKey)
{
    word32 keyLength = 48;
    uint8_t salt[16];
    RAND_bytes(salt, 16);
    uint8_t sharedSecret[48];
    uint8_t secretKeyBytes[32];
    ecc_key playerPublicKey;
    wc_ecc_init(&playerPublicKey);
    word32 idx = 0;
    if (wc_EccPublicKeyDecode(reinterpret_cast<const byte*>(playerKey.data()), &idx, &playerPublicKey, playerKey.size()) != 0
        || wc_ecc_shared_secret(&serverKey, &playerPublicKey, sharedSecret, &keyLength) != 0) {
        wc_ecc_free(&playerPublicKey);
        return false;
    }
    wc_ecc_free(&playerPublicKey);
    wc_Sha256 sha256;
    wc_InitSha256(&sha256);
    wc_Sha256Update(&sha256, salt, 16);
    wc_Sha256Update(&sha256, sharedSecret, keyLength);
    wc_Sha256Final(&sha256, secretKeyBytes);
    wc_Sha256Free(&sha256);
    uint8_t iv[16];
    memcpy(iv, secretKeyBytes, 12);
    iv[12] = 0;
    iv[13] = 0;
    iv[14] = 0;
    iv[15] = 0x02;
    wc_AesInit(&aes.encryptBlock, nullptr, INVALID_DEVID);
    wc_AesInit(&aes.decryptBlock, nullptr, INVALID_DEVID);
    wc_AesSetKeyDirect(&aes.encryptBlock, secretKeyBytes, 32, iv, AES_ENCRYPTION);
    wc_AesSetKeyDirect(&aes.decryptBlock, secretKeyBytes, 32, iv, AES_DECRYPTION);
    memcpy(aes.key, secretKeyBytes, 32);
    aes.isEncryptionEnabled = true;
    return true;
}

bool encrypt(AES& aes, const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output)
{
    const uint64_t hash = calculateChecksum(aes.encryptBlockCounter++, aes.key, input, inputSize);
    std::vector<uint8_t> buffer;
    buffer.reserve(inputSize + 8);
    buffer.insert(buffer.end(), input, input + inputSize);
    io::BinaryWriter bi{};
    bi.writeU64LE(hash);
    buffer.insert(buffer.end(), bi.data(), bi.data() + 8);
    output.resize(buffer.size());
    wc_AesCtrEncrypt(&aes.encryptBlock, output.data(), buffer.data(), buffer.size());
    return true;
}

bool decrypt(AES& aes, const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output)
{
    output.resize(inputSize);
    wc_AesCtrEncrypt(&aes.decryptBlock, output.data(), input, inputSize);
    io::BinaryReader ri(output.data() + inputSize - 8, 8);
    const uint64_t receivedHash = ri.readU64LE();
    const uint64_t calculatedHash = calculateChecksum(aes.decryptBlockCounter++, aes.key, output.data(), inputSize - 8);
    if (calculatedHash != receivedHash) {
        return false;
    }
    output.resize(inputSize - 8);
    return true;
}
}