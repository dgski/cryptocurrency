#include "shared/Crypto.h"
#include "shared/Types.h"

int main()
{
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    ERR_load_crypto_strings();
    ERR_free_strings();

    str data = "This is some random data!";
    str privateKeyFileName = "pri_0.pem";
    str publicKeyFileName = "pub_0.pem";

    RSAKeyPair keys(privateKeyFileName, publicKeyFileName);

    if(!keys.isValid())
    {
        log("Could not create keys!");
        return -1;
    }

    log("keys created!:");

    str signature = keys.signData(data.c_str(), data.length());

    log("signature: %", signature);


    RSAKeyPair pubKey(keys.publicKey);

    const bool valid = pubKey.isSignatureValid(data.c_str(), data.length(), signature);
    log("isSigatureValid: %", valid? "true" : "false");
}