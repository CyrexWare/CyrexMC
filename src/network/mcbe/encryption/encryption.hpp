#pragma once

#include <array>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/ecc.h>

#include <cassert>
#include <cstdint>

namespace cyrex::network::mcbe::encryption
{


struct AesEncryptor
{
    struct EccKey : public ecc_key
    {
        EccKey()
        {
            assert(wc_ecc_init(this) == 0);
        }

        ~EccKey()
        {
            assert(wc_ecc_free(this) == 0);
        }

        EccKey(const EccKey&) = delete;
        EccKey& operator=(const EccKey&) = delete;

        EccKey(EccKey&&) = delete;
        EccKey& operator=(EccKey&&) = delete;
    };
    using EccKeyPtr = std::unique_ptr<EccKey>;

    struct AesBlock : public Aes
    {
        std::int64_t counter;

        AesBlock()
        {
            assert(wc_AesInit(this, nullptr, INVALID_DEVID) == 0);
        }

        ~AesBlock()
        {
            wc_AesFree(this);
        }

        AesBlock(const AesBlock&) = delete;
        AesBlock& operator=(const AesBlock&) = delete;

        AesBlock(AesBlock&&) = delete;
        AesBlock& operator=(AesBlock&&) = delete;
    };
    using AesBlockPtr = std::unique_ptr<AesBlock>;

    std::int64_t encryptBlockCounter = 0;
    std::int64_t decryptBlockCounter = 0;

    EccKey* serverKey;
    AesBlockPtr encryptBlock = std::make_unique<AesBlock>();
    AesBlockPtr decryptBlock = std::make_unique<AesBlock>();

    std::array<uint8_t, 32> key{};
    std::array<uint8_t, 16> salt{};

    AesEncryptor(EccKey* serverKey, std::string_view playerKey) : serverKey(serverKey)
    {
        word32 idx = 0;
        EccKey playerPublicKey;
        if (wc_EccPublicKeyDecode(reinterpret_cast<const byte*>(playerKey.data()), &idx, &playerPublicKey, playerKey.size()))
        {
            throw std::runtime_error("couldn't init ecc  key");
        }

        word32 sharedSecretLength = 0;
        std::array<uint8_t, 48> sharedSecret{};
        if (wc_ecc_shared_secret(serverKey, &playerPublicKey, sharedSecret.data(), &sharedSecretLength) != 0)
        {
            throw std::runtime_error("couldn't init ecc  key");
        }
        RAND_bytes(salt.data(), salt.size());

        wc_Sha256 sha256;
        wc_InitSha256(&sha256);
        wc_Sha256Update(&sha256, salt.data(), salt.size());
        wc_Sha256Update(&sha256, sharedSecret.data(), sharedSecretLength);
        wc_Sha256Final(&sha256, key.data());

        std::array<::uint8_t, 16> iv{};
        std::ranges::copy(key, iv.begin());
        iv.back() = 0x02;

        wc_AesSetKeyDirect(encryptBlock.get(), key.data(), key.size(), iv.data(), AES_ENCRYPTION);
        wc_AesSetKeyDirect(decryptBlock.get(), key.data(), key.size(), iv.data(), AES_DECRYPTION);
    }

    [[nodiscard]] std::optional<std::vector<uint8_t>> encrypt(std::span<const uint8_t> input) const;
    [[nodiscard]] std::optional<std::vector<uint8_t>> decrypt(std::span<const uint8_t> input) const;
};


} // namespace cyrex::network::mcbe::encryption
