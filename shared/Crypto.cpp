#include "Crypto.h"

std::optional<RSAKeyPair> RSAKeyPair::create(const str& privateKeyFilename, const str& publicKeyFilename)
{
    RSAKeyPair keys;

    keys.rsa = RSA_new();

    FILE* pri_key_file = fopen(privateKeyFilename.c_str(), "r");
    if(pri_key_file == NULL)
    {
        return std::nullopt;
    }
    keys.rsa = PEM_read_RSAPrivateKey(pri_key_file, &keys.rsa, NULL, NULL);
    fclose(pri_key_file);
    
    FILE* pub_key_file = fopen(publicKeyFilename.c_str(), "r");
    if(pub_key_file == NULL)
    {
        return std::nullopt;
    }
    keys.rsa = PEM_read_RSAPublicKey(pub_key_file, &keys.rsa, NULL, NULL);
    fclose(pub_key_file);

    std::ifstream t(publicKeyFilename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    keys.publicKey = std::move(buffer.str());

    return std::move(keys);
}

std::optional<RSAKeyPair> RSAKeyPair::create(const str& publicKey)
{
    RSAKeyPair keys;
    
    BIO* bio = BIO_new_mem_buf((void*)publicKey.c_str(), publicKey.length());
    keys.rsa = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);

    return std::move(keys);
}

str RSAKeyPair::signData(const void* data, size_t dataLen) const
{
    unsigned char sig[RSA_size(rsa)];
    unsigned int sig_len = 0;
    const int res = RSA_sign(
        NID_sha256,
        (const unsigned char*)data,
        dataLen,
        sig,
        &sig_len,
        rsa
    );

    if(res != 1)
    {
        throw std::runtime_error("Could not sign data");
    }

    str signatureStr;
    for(unsigned char c : sig)
    {
        char buf[3] = {0};
        sprintf(buf,"%02x", c);
        signatureStr += buf;
    }

    return signatureStr;
}

bool RSAKeyPair::isSignatureValid(const void* data, size_t dataLen, const str& signature) const
{
    unsigned char sig[256];
    const char* pointer = signature.c_str();
    for(int i = 0; i < 256; i++)
    {
        unsigned int value;
        sscanf(pointer, "%02x", &value);
        pointer = pointer + 2;
        sig[i] = value;
    }

    const int res = RSA_verify(
        NID_sha256,
        (const unsigned char*)data,
        dataLen,
        sig,
        256,
        rsa
    );

    if(res == 0)
    {
        return false;
    }

    return true;
}