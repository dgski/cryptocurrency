#include "Miner.h"

Miner::Miner(const char* ip, const char* port)
{
    std::cout << "Miner ip=" << ip << " port=" << port << std::endl;
    sockaddr_in serv_addr;

    incomingFromManager = socket(AF_INET, SOCK_STREAM, 0);
    if (incomingFromManager < 0)
    { 
        std::cout << "Socket creation error" << std::endl;
        return;
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(atoi(port)); 
       
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)  
    { 
        std::cout << "Invalid address/ Address not supported" << std::endl;
        return; 
    } 
   
    if (connect(incomingFromManager, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        std::cout << "Connection Failed" << std::endl;
        return; 
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    setsockopt(incomingFromManager, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(incomingFromManager, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
}

void Miner::run()
{
    while(true)
    {
        std::optional<Message> msg = getMessage(incomingFromManager);
        if(msg.has_value())
        {
            Parser parser(msg.value());
            if(msg.value().id == 1)
            {
                u64 newBaseHash;
                parser.parse_u64(newBaseHash);
                baseHash.set(newBaseHash);
                std::cout << "Received New Base Hash:" << newBaseHash << std::endl;
                if(!currentlyMining)
                {
                    startMining();
                }
            }
        }

        if(proof.ready())
        {
            u64 proofValue = proof.get();
            stopMining();
            std::cout << "Sending Proof of Work To Manager Module: " << proofValue << std::endl;

            Message msg;
            msg.id = 2;
            msg.compose_u64(proofValue);
            sendMessage(incomingFromManager, msg);
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