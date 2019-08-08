#include "Manager.h"

Manager::Manager(const char* iniFileName)
{
    std::cout << "Manager Module Starting" << std::endl;

    const std::map<str,str> params = getInitParameters(iniFileName);

    connFromMiners.init(
        params.at("connFromMinersIP").c_str(),
        atoi(params.at("connFromMinersPORT").c_str())
    );

    connFromTransactioner.init(
        params.at("connFromTransactionerIP").c_str(),
        atoi(params.at("connFromTransactionerPORT").c_str()) 
    );
    
    currentBaseHash = 12393939334343; // FAKE
}

void Manager::run()
{
    connFromTransactioner.acceptNewConnections(true);

    while(true)
    {
        // Connect to New Miners
        const std::vector<int> newConnections = connFromMiners.acceptNewConnections();
        sendNewBaseHashToMiners(newConnections);
        
        // Check if any miner has sent the proof of work
        for(int s : connFromMiners.sockets)
        {
            std::optional<Message> msg = getMessage(s);
            if(msg.has_value())
            {
                processMinerMessage(msg.value());
            }
        }
        
        // Ask for new transactions
        if(postedTransactions.size() < 100)
        {
            askTransactionerForNewTransactions();
        }

        // Transactioner sent new transactions
        for(int socket : connFromTransactioner.sockets)
        {
            std::optional<Message> msg = getMessage(socket);
            if(msg.has_value())
            {
                processTransactionerMessage(msg.value());
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Manager::processMinerMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_MINER_MANAGER_PROOFOFWORK::id:
    {
        MSG_MINER_MANAGER_PROOFOFWORK contents;
        Parser parser(msg);
        contents.parse(parser);

        std::cout << "Received Proof of Work:" << contents.proofOfWork << std::endl;
        return;
    }
    }
}

void Manager::processTransactionerMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_A_MANAGER_TRANSACTIONER_TRANSREQ::id:
    {
        MSG_A_MANAGER_TRANSACTIONER_TRANSREQ contents;
        Parser parser(msg);
        contents.parse(parser);
        
        for(Transaction& t : contents.transactions)
        {
            postedTransactions.push_back(t);
        }

        u64 newBaseHash = hashVector(postedTransactions);
        if(newBaseHash != currentBaseHash)
        {

            currentBaseHash = newBaseHash;
            sendNewBaseHashToMiners(connFromMiners.sockets);
        }
        return;
    }
    }
}

template<typename T>
size_t Manager::hashVector(std::vector<T> data)
{
    size_t seed = data.size();
    for(auto d : data)
    {
        seed ^= 3203302344 ^ std::hash<T>{}(d);
    }

    return seed;
}

void Manager::sendNewBaseHashToMiners(const std::vector<int>& sockets) const
{
    for(int socket : sockets)
    {
        MSG_MANAGER_MINER_NEWBASEHASH contents;
        contents.newBaseHash = currentBaseHash;

        Message reply;
        contents.compose(reply);
        sendMessage(socket, reply);
    }
}

void Manager::askTransactionerForNewTransactions() const
{
    for(int socket : connFromTransactioner.sockets)
    {
        MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents;
        contents.numOfTransactionsRequested = 5;
        
        Message msg;
        contents.compose(msg);
        sendMessage(socket, msg);
    }
}