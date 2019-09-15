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
        parser.parse(id, hashOfLastBlock, proofOfWork, transactions);
    }

    void compose(Message& msg) const
    {
        msg.compose(id, hashOfLastBlock, proofOfWork, transactions);
    }

    bool isValid()
    {
        u64 baseHash = calculateBaseHash();
        if(!validProof(proofOfWork, baseHash))
        {
            return false;
        }

        for(const Transaction& t : transactions)
        {
            if(!t.isSignatureValid())
            {
                return false;
            }
        }

        return true;
    }
};