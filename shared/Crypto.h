#pragma once

#include <fstream>
#include <sstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Utils.h"

class RSAKeyPair
{
    RSA* rsa;
    bool valid = false;

    RSAKeyPair

public:
    str publicKey;
    RSAKeyPair(const str& privateKeyFilename, const str& publicKeyFilename);
    RSAKeyPair(const str& publicKey);
    str signData(const void* data, size_t dataLen) const;
    bool isSignatureValid(const void* data, size_t dataLen, const str& signature) const;
    bool isValid();

    ~RSAKeyPair()
    {
        RSA_free(rsa);
    }
};