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
    
    registerServerConnection(&connFromMiners);
    registerServerConnection(&connFromTransactioner);

    registerRepeatedTask([this]()
    {
        if(postedTransactions.size() < 100)
        {
            askTransactionerForNewTransactions();
        }
    });

    registerRepeatedTask([]()
    {
        std::this_thread::sleep_for (std::chrono::seconds(1));
    });

    currentBaseHash = 12393939334343; // FAKE
}

void Manager::askTransactionerForNewTransactions()
{    
    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents;
    contents.numOfTransactionsRequested = 200 - postedTransactions.size();

    Message msg;
    contents.compose(msg);

    connFromTransactioner.sendMessage(msg, [this](Message& reply)
    {
        processTransactionRequestReply(reply);
    });
}

void Manager::processTransactionRequestReply(Message& msg)
{
    std::cout << "processTransactionRequestReply" << std::endl;

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

        MSG_MANAGER_MINER_NEWBASEHASH contents;
        contents.newBaseHash = currentBaseHash;
        Message hashMsg;
        contents.compose(hashMsg);
        connFromMiners.sendMessage(hashMsg);
    }
    return;
}

void Manager::processMessage(Message& msg)
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
            exit(0);
        }

        return;
    }
    case MSG_MINER_MANAGER_HASHREQUEST::id:
    {
        MSG_MANAGER_MINER_NEWBASEHASH contents;
        contents.newBaseHash = currentBaseHash;
        Message hashMsg;
        contents.compose(hashMsg);
        connFromMiners.sendMessage(msg.socket, hashMsg);
    }
    }
}