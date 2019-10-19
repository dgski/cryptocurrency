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

    void sign(const RSAKeyPair& keys)
    {
        size_t hashValue{ Transaction::hashValue(*this) };
        signature = move(keys.signData(&hashValue, sizeof(size_t)));
    }

    bool isSignatureValid() const
    {
        logger.logInfo("About the check trans!");
        size_t hashValue{ Transaction::hashValue(*this) };
        auto keys = RSAKeyPair::create(sender);
        return keys.value().isSignatureValid(&hashValue, sizeof(size_t), signature);
    }

    static size_t hashValue(const Transaction& transaction)
    {
        return
            std::hash<u64>{}(transaction.time) ^
            std::hash<str>{}(transaction.sender) ^
            std::hash<str>{}(transaction.recipiant) ^
            std::hash<u64>{}(transaction.amount);
    }
};