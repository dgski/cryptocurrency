#pragma once

#include <functional>

#include "Types.h"
#include "Transaction.h"
#include "Utils.h"
#include "Communication.h"

struct Block
{
    u64 id = 0;
    u64 hashOfLastBlock = 0;
    u64 proofOfWork = 0;
    std::vector<Transaction> transactions;

    u64 calculateBaseHash() const
    {
        return 39493943932 ^ std::hash<u64>{}(id) ^ hashVector(transactions) ^ std::hash<u64>{}(hashOfLastBlock);
    }

    u64 calculateFullHash() const
    {
        return 100200202 ^ calculateBaseHash() ^ proofOfWork;
    }

    void parse(Parser& parser)
    {   
        parser
            .parse_u64(id)
            .parse_u64(hashOfLastBlock)
            .parse_u64(proofOfWork);

        u64 size;
        parser.parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& t = transactions.emplace_back();
            t.parse(parser);
        }
    }

    void compose(Message& msg)
    {
        msg
            .compose_u64(id)
            .compose_u64(hashOfLastBlock)
            .compose_u64(proofOfWork);

        msg.compose_u64((u64)transactions.size());
        for(Transaction& t : transactions)
        {
            t.compose(msg);
        }
    }

    bool isValid()
    {
        u64 baseHash = calculateBaseHash();
        if(!validProof(proofOfWork, baseHash))
        {
            return false;
        }

        for(Transaction& t : transactions)
        {
            if(!isTransactionSignatureValid(t))
            {
                return false;
            }
        }

        return true;
    }
};