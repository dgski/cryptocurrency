#include "Miner.h"

Miner::Miner()
{
    struct sockaddr_in serv_addr; 
    if ((incomingFromManager = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return;
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(6001); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return; 
    } 
   
    if (connect(incomingFromManager, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return; 
    }
}

void Miner::run()
{
    while(true)
    {
        std::optional<Message> msg = getMessage();

        if(0 /* NEW_HASH */)
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

std::optional<Message> Miner::getMessage()
{
    Message msg;

    int valread = read(incomingFromManager , &msg.id, 4);

    if(valread == 0)
        return std::nullopt;

    valread = read(incomingFromManager, &msg.size, 8);
    msg.data.resize(msg.size);
    valread = read(incomingFromManager, msg.data.data(), (size_t)msg.size);

    std::cout << "Msg: id=" << msg.id << " size=" << msg.size << std::endl;

    for(byte b : msg.data)
        std::cout << (int)b << std::endl;

    std::cout << "Contents: " << *(u64*)(msg.data.data()) << std::endl;

    return msg;
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