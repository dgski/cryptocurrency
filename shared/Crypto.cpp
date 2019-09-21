#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Crypto.h"

static bool cryptoInitialized = false;

str rsa_signData(const void* data, size_t dataLen, const str& privateKeyFilename, const str& publicKeyFilename)
{
    if(!cryptoInitialized)
    {
        OpenSSL_add_all_algorithms();
        OpenSSL_add_all_ciphers();
        ERR_load_crypto_strings();
        ERR_free_strings();
        cryptoInitialized = true;
    }

    log("rsa_signData: signing data, len=%", dataLen);

    RSA* rsa = RSA_new();

   //Read private key
    FILE* pri_key_file = fopen(privateKeyFilename.c_str(), "r");
    if(pri_key_file == NULL) return 0;

    rsa = PEM_read_RSAPrivateKey(pri_key_file, &rsa, NULL, NULL);
    fclose(pri_key_file);

    printf("\n");
    
    //Read public key
    FILE* pub_key_file = fopen(publicKeyFilename.c_str(), "r");
    if(pub_key_file == NULL) return 0;

    rsa = PEM_read_RSAPublicKey(pub_key_file, &rsa, NULL, NULL);

    fclose(pub_key_file);

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
        log("rsa_signData: Could not sign data!");
        return {};
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

bool rsa_isSignatureValid(const void* data, size_t dataLen, const str& publicKey, const str& signature)
{
    if(!cryptoInitialized)
    {
        OpenSSL_add_all_algorithms();
        OpenSSL_add_all_ciphers();
        ERR_load_crypto_strings();
        ERR_free_strings();
        cryptoInitialized = true;
    }

    //BIO* bio = BIO_new_mem_buf((void*)publicKey.c_str(), publicKey.length());
    //RSA* rsa_pub = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);

    //Read public key
    RSA* rsa = RSA_new();
    FILE* pub_key_file = fopen(publicKey.c_str(), "r");
    if(pub_key_file == NULL) return 0;
    rsa = PEM_read_RSAPublicKey(pub_key_file, &rsa, NULL, NULL);
    fclose(pub_key_file);

    log("step 1");

    unsigned char data2[32] = { 0 };
    unsigned char sig[256];
    const char* pointer = signature.c_str();
    //extract sig from hex asci
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

    log("step 2");

    //BIO_free_all(bio);
    RSA_free(rsa);

    if(res == 0)
    {
        return false;
    }

    return true;
}