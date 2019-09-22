#pragma once

#include <optional>
#include <fstream>
#include <sstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Utils.h"

class RSAKeyPair
{
    RSA* rsa = nullptr;
    RSAKeyPair(){}
public:
    RSAKeyPair(const RSAKeyPair&) = delete;
    RSAKeyPair& operator=(const RSAKeyPair&) = delete;
    RSAKeyPair(RSAKeyPair&& other)
    : rsa(other.rsa),
    publicKey(std::move(other.publicKey))
    {
        other.rsa = nullptr;
    }
    RSAKeyPair& operator=(RSAKeyPair&& other)
    {
        rsa = other.rsa;
        other.rsa = nullptr;
        publicKey = std::move(other.publicKey);

        return *this;
    }

    ~RSAKeyPair()
    {
        if(rsa != nullptr)
        {
            RSA_free(rsa);
        }
    }

    str publicKey;
    static std::optional<RSAKeyPair> create(const str& privateKeyFilename, const str& publicKeyFilename);
    static std::optional<RSAKeyPair> create(const str& publicKey);
    str signData(const void* data, size_t dataLen) const;
    bool isSignatureValid(const void* data, size_t dataLen, const str& signature) const;
};