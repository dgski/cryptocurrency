#include "Miner.h"

void Miner::run()
{
    while(true)
    {
        i32 c = getMessage();

        if(c == 1 /* NEW_HASH_FOUND */)
        {
            baseHash.set(29392939343);            
            if(currentlyMining == false)
            {
                startMining();
            }
        }

        if(proof.ready())
        {
            u64 proofValue = proof.get();
            stopMining();
            std::cout << "Sending Proof of Work To Manager Module: " << proofValue << std::endl;
        }
    }
}

i32 Miner::getMessage()
{
    i32 msgId;
    std::cout << "Enter Fake Message Id:" << std::endl;
    std::cin >> msgId;
    std::cout << std::endl;
    return msgId;  
}

bool Miner::validProof(u64 nonce, u64 hash) const
{
    const size_t res = std::hash<u64>{}(nonce) ^ std::hash<u64>{}(hash);
    const size_t mask = (size_t)(0b11111111111111111111111111111111);
    return ((mask ^ res) & mask) == mask;
}

void Miner::startMining()
{
    currentlyMining = true;
    std::thread t(&Miner::mine, this);
    t.detach();
}

void Miner::stopMining()
{
    currentlyMining = false;
}

void Miner::mine()
{
    u64 nonce = 0;
    u64 currentBaseHash = baseHash.get();

    while(true)
    {
        const bool res = validProof(nonce, currentBaseHash);
        
        if(baseHash.ready())
        {
            currentBaseHash = baseHash.get();
            continue;
        }

        if(res)
        {
            proof.set(nonce);
            return;
        }

        nonce += 1;
    }
}