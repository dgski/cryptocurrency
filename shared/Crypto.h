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
    RSA* rsa;
public:
    str publicKey;

    static std::optional<RSAKeyPair> create(const str& privateKeyFilename, const str& publicKeyFilename);
    static std::optional<RSAKeyPair> create(const str& publicKey);
    str signData(const void* data, size_t dataLen) const;
    bool isSignatureValid(const void* data, size_t dataLen, const str& signature) const;
};