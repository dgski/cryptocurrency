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
        parser
            .parse_u64(time)
            .parse_str(sender)
            .parse_str(recipiant)
            .parse_u64(amount)
            .parse_str(signature);
    }

    void compose(Message& msg) const
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

    void sign(const str& privateKey)
    {
        signature = move(rsa_signData(this, sizeof(Transaction) - sizeof(str), privateKey));
    }

    bool isSignatureValid() const
    {
        return rsa_isSignatureValid(this, sizeof(Transaction) - sizeof(str), sender, signature);
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