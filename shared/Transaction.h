#pragma once

#include "Types.h"
#include "Communication.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

struct Transaction
{
    u64 time;
    str sender;
    str recipiant;
    u64 amount;
    str signature;

    void parse(Parser& parser)
    {
        parser
            .parse_u64(time)
            .parse_str(sender)
            .parse_str(recipiant)
            .parse_u64(amount)
            .parse_str(signature);
    }

    void compose(Message& msg)
    {
        msg
            .compose_u64(time)
            .compose_str(sender)
            .compose_str(recipiant)
            .compose_u64(amount)
            .compose_str(signature);
    }

    void logTransaction() const
    {
        log(
            "Transaction"
            "{ time:%, sender:%, recipiant:%, amount:%, signature:% }",
            time,
            sender,
            recipiant,
            amount,
            signature
        );
    }
};

namespace std
{
    template<>
    struct hash<Transaction>
    {
        typedef Transaction& argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& transaction) const noexcept
        {
            return
                std::hash<u64>{}(transaction.time) ^
                std::hash<str>{}(transaction.sender) ^
                std::hash<str>{}(transaction.recipiant) ^
                std::hash<u64>{}(transaction.amount) ^
                std::hash<str>{}(transaction.signature);
        }
    };
}

inline bool isTransactionSignatureValid(Transaction& transaction)
{
    log("isTransactionSignatureValid");

    return true;

    str transHash = std::to_string(std::hash<Transaction>{}(transaction));

    char our_key[1000] = {0};
    char* new_key_point = our_key;
    const char* send_pointer = transaction.sender.c_str();

    /*
    for(int i = 0; i < 5; i++)
    {
        memcpy(new_key_point,send_pointer,64);
        new_key_point = new_key_point + 64;
        send_pointer = send_pointer + 64;
        *new_key_point++ = '\n';
    }

    memcpy(new_key_point,send_pointer,41);
    */
   
    char final_key[427] = {0};
    sprintf(final_key,"-----BEGIN RSA PUBLIC KEY-----\n%s\n-----END RSA PUBLIC KEY-----\n", our_key);
    char* pub_key = final_key;

    BIO *bio = BIO_new_mem_buf((void*)pub_key, strlen(pub_key));
    RSA *rsa_pub = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);

    int transactionIsValid = RSA_verify(
        NID_sha256,
        (const unsigned char*) transHash.c_str(),
        transHash.size(),
        (const unsigned char*) transaction.signature.c_str(),
        transaction.signature.size(),
        rsa_pub
    );

    BIO_free_all(bio);
    RSA_free(rsa_pub);

    return transactionIsValid != 0;
}

inline void signTransaction(Transaction& transaction, RSA* keypair)
{
    str transHash = std::to_string(std::hash<Transaction>{}(transaction));

    unsigned char sig[RSA_size(keypair)];
    unsigned int sig_len = 0;
    int res = RSA_sign(
        NID_sha256,
        (const unsigned char*) transHash.c_str(),
        transHash.size(),
        sig,
        &sig_len,
        keypair
    );
    if(res != 1)
    {
        return;
    }

    for(int i = 0; i < 256; i++)
    {
        char buf[3] = {0};
        sprintf(buf,"%02x", sig[i]);
        transaction.signature += buf;
    }
}