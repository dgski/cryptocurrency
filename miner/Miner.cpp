#include "Miner.h"

Miner::Miner(const char* iniFileName)
{
    std::cout << "Miner Module Starting" << std::endl;

    std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(
        params["connToManagerIP"].c_str(),
        atoi(params["connToManagerPORT"].c_str())
    );
}

void Miner::run()
{
    while(true)
    {
        std::optional<Message> msg = getMessage(connToManager.getSocket());
        if(msg.has_value())
        {
            processManagerMessage(msg.value());
        }

        if(proof.ready())
        {
            u64 proofValue = proof.get();
            stopMining();
            std::cout << "Sending Proof of Work To Manager Module: " << proofValue << std::endl;

            MSG_MINER_MANAGER_PROOFOFWORK contents;
            contents.proofOfWork = proofValue;

            Message msg;
            contents.compose(msg);
            sendMessage(connToManager.getSocket(), msg);
            return;
        }

        std::this_thread::sleep_for (std::chrono::milliseconds(1));
    }
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

void Miner::processManagerMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_MANAGER_MINER_NEWBASEHASH::id:
    {
        MSG_MANAGER_MINER_NEWBASEHASH contents;
        Parser parser(msg);
        contents.parse(parser);
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
