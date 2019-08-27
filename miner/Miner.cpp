#include "Miner.h"

Miner::Miner(const char* iniFileName)
{
    log("Miner Module Starting");

    const std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    registerClientConnection(&connToManager);

    log("Requesting hash");
    MSG_MINER_MANAGER_HASHREQUEST hashRequest;
    Message msg;
    hashRequest.compose(msg);
    connToManager.sendMessage(msg);

    registerRepeatedTask([this]()
    {
        checkProof();
    });
}

void Miner::processMessage(const Message& msg)
{
    switch(msg.id)
    {
        case MSG_MANAGER_MINER_NEWBASEHASH::id:
        {
            processManagerNewBaseHash(msg);
            return;
        }
        default:
        {
            log("Unhandled MSG id=%", msg.id);
            return;
        }
    }
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

void Miner::checkProof()
{
    if(proof.ready())
    {
        u64 proofValue = proof.get();
        stopMining();
        log("Found valid proof=%, Sending to Manager", proofValue);

        MSG_MINER_MANAGER_PROOFOFWORK contents;
        contents.proofOfWork = proofValue;
        Message msg;
        contents.compose(msg);

        connToManager.sendMessage(msg);
        return;
    }
}

void Miner::processManagerNewBaseHash(const Message& msg)
{
    MSG_MANAGER_MINER_NEWBASEHASH contents{ msg };
    baseHash.set(contents.newBaseHash);

    log("Recieved new baseHash: %", contents.newBaseHash);
    if(!currentlyMining)
    {
        startMining();
    }
}