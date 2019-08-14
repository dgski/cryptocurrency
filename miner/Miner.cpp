#include "Miner.h"

Miner::Miner(const char* iniFileName)
{
    std::cout << "Miner Module Starting" << std::endl;

    std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(
        params["connToManagerIP"].c_str(),
        atoi(params["connToManagerPORT"].c_str())
    );

    registerClientConnection(&connToManager);

    MSG_MINER_MANAGER_HASHREQUEST hashRequest;
    Message msg;
    hashRequest.compose(msg);
    connToManager.sendMessage(msg);

    registerRepeatedTask([this]()
    {
        if(proof.ready())
        {
            u64 proofValue = proof.get();
            stopMining();
            std::cout << "Sending Proof of Work To Manager Module: " << proofValue << std::endl;

            MSG_MINER_MANAGER_PROOFOFWORK contents;
            contents.proofOfWork = proofValue;

            Message msg;
            contents.compose(msg);
            connToManager.sendMessage(msg);
            return;
        }
    });
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
    bool res;

    while(true)
    {
        res = validProof(nonce, currentBaseHash);
        
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

void Miner::processMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_MANAGER_MINER_NEWBASEHASH::id:
    {
        MSG_MANAGER_MINER_NEWBASEHASH contents{ msg };
        baseHash.set(contents.newBaseHash);

        std::cout << "Received New Base Hash:" << contents.newBaseHash << std::endl;
        if(!currentlyMining)
        {
            startMining();
        }
        return;
    }
    }
}