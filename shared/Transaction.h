#pragma once

#include "Types.h"
#include "Communication.h"
#include "Crypto.h"

struct Transaction
{
    u64 time;
    str sender;
    str recipiant;
    u64 amount;
    str signature;

    void parse(Parser& parser)
    {
        parser.parse(time, sender, recipiant, amount, signature);
    }

    void compose(Message& msg) const
    {
        msg.compose(time, sender, recipiant, amount, signature);
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

    void sign(const str& privateKey)
    {
        size_t hashValue{ Transaction::hashValue(*this) };

        signature = move(rsa_signData(&hashValue, sizeof(size_t), privateKey));
    }

    bool isSignatureValid() const
    {
        size_t hashValue{ Transaction::hashValue(*this) };
        
        return rsa_isSignatureValid(this, sizeof(Transaction) - sizeof(str), sender, signature);
    }

    static size_t hashValue(const Transaction& transaction)
    {
        return
            std::hash<u64>{}(transaction.time) ^
            std::hash<str>{}(transaction.sender) ^
            std::hash<str>{}(transaction.recipiant) ^
            std::hash<u64>{}(transaction.amount) ^
            std::hash<str>{}(transaction.signature);
    }
};