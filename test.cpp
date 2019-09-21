#include "shared/Crypto.h"
#include "shared/Types.h"

int main()
{
    str data = "This is some random data!";
    str privateKeyFileName = "pri_0.pem";
    str publicKeyFileName = "pub_0.pem";

    str signature = rsa_signData(data.c_str(), data.length(), privateKeyFileName, publicKeyFileName);

    log("signature: %", signature);


    


    const bool valid = rsa_isSignatureValid(data.c_str(), data.length(), publicKeyFileName, signature);

    log("isSigatureValid: %", valid? "true" : "false");
}