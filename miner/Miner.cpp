#include "Miner.h"

Miner::Miner(const char* iniFileName)  : Module("miner")
{
    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);
    
    logger.logInfo("Miner Module Starting");

    connToManager.init(strToIp(params.at("connToManager")));
    registerConnections({&connToManager});

    incrementSize = atoi(params.at("incrementSize").c_str());

    requestNewBaseHash();
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
            processUnhandledMessage(msg);
            return;
        }
    }
}

void Miner::startMining()
{
    logger.logInfo("Starting mining");

    currentlyMining = true;
    miningThread = std::make_unique<std::thread>(&Miner::mine, this);
    registerScheduledTask(ONE_SECOND, [this]()
    {
        checkProof();
    });
}

void Miner::stopMining()
{
    logger.logInfo("Stopping mining");

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

        nonce += incrementSize;
    }
}

void Miner::checkProof()
{
    if(proof.ready())
    {
        const u64 proofValue = proof.get();
        stopMining();
        logger.logInfo({
            {"event", "Found valid proof. Sending to Manager"},
            {"proof", proofValue}
        });

        MSG_MINER_MANAGER_PROOFOFWORK outgoing;
        outgoing.proofOfWork = proofValue;

        connToManager.sendMessage(outgoing.msg());
        return;
    }

    registerScheduledTask(ONE_SECOND, [this]()
    {
        checkProof();
    });
}

void Miner::processManagerNewBaseHash(const Message& msg)
{
    MSG_MANAGER_MINER_NEWBASEHASH incoming{ msg };
    
    baseHash.set(incoming.newBaseHash);
    if(miningThread && miningThread->joinable())
    {
        miningThread->join();
    }
    if(!currentlyMining)
    {
        startMining();
    }
}

void Miner::requestNewBaseHash()
{
    logger.logInfo("Requesting new base hash");
    MSG_MINER_MANAGER_HASHREQUEST outgoing;
    connToManager.sendMessage(outgoing.msg());
}