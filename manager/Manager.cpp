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
        //for(int s : connFromMiners.sockets)
        //{
        //    std::optional<Message> msg = getFinalMessage(s);
        //    if(msg.has_value())
        //    {
        //        processMinerMessage(msg.value());
        //    }
        //}
        
        // Ask for new transactions
        if(postedTransactions.size() < 100)
        {
            askTransactionerForNewTransactions();
        }

        std::optional<Message> msg = connFromTransactioner.getMessage();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Manager::processMinerMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_MINER_MANAGER_PROOFOFWORK::id:
    {
        MSG_MINER_MANAGER_PROOFOFWORK contents{ msg };

        if(validProof(contents.proofOfWork, currentBaseHash))
        {
            // Propagate to Network
            // sendMessage(connToNetworked.getSocket(), msg);
            
            // Save to Blockchainer module
            // sendMessage(connToBlockchainer.getSocket(), msg);
            std::cout << "Received Proof of Work:" << contents.proofOfWork << std::endl;
        }

        return;
    }
    }
}

void Manager::sendNewBaseHashToMiners(const std::vector<int>& sockets) const
{
    for(int socket : sockets)
    {
        MSG_MANAGER_MINER_NEWBASEHASH contents;
        contents.newBaseHash = currentBaseHash;

        Message reply;
        contents.compose(reply);
        sendFinalMessage(socket, reply);
    }
}

void Manager::askTransactionerForNewTransactions()
{
    std::cout << "Asking for new Transactions" << std::endl;
    
    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents;
    contents.numOfTransactionsRequested = 5;

    Message msg;
    contents.compose(msg);

    connFromTransactioner.sendMessage(msg, [this](Message& msg)
    {
        std::cout << "Callback!" << std::endl;
        processTransactionRequestReply(msg);
    });
}

void Manager::processTransactionRequestReply(Message& msg)
{
    std::cout << "inside vallback!" << std::endl;
    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };

    std::cout << "Got " << contents.transactions.size() << " Transactions" << std::endl;
    
    for(Transaction& t : contents.transactions)
    {
        postedTransactions.push_back(t);
    }

    u64 newBaseHash = hashVector(postedTransactions);
    if(newBaseHash != currentBaseHash)
    {
        currentBaseHash = newBaseHash;
        std::cout << "baseHash has changed!" << std::endl;
        //sendNewBaseHashToMiners(connFromMiners.sockets);
    }
    return;
}