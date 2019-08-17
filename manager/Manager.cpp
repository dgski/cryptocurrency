#include "Manager.h"

Manager::Manager(const char* iniFileName)
{
    const std::map<str,str> params = getInitParameters(iniFileName);

    log("Manager module starting");

    connFromMiners.init(
        params.at("connFromMinersIP").c_str(),
        atoi(params.at("connFromMinersPORT").c_str())
    );

    connFromTransactioner.init(
        params.at("connFromTransactionerIP").c_str(),
        atoi(params.at("connFromTransactionerPORT").c_str()) 
    );

    connFromNetworker.init(
        params.at("connFromNetworkerIP").c_str(),
        atoi(params.at("connFromNetworkerPORT").c_str())
    );
    
    registerServerConnection(&connFromMiners);
    registerServerConnection(&connFromTransactioner);
    registerServerConnection(&connFromNetworker);

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

void Manager::processMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_MINER_MANAGER_PROOFOFWORK::id:
    {
        MSG_MINER_MANAGER_PROOFOFWORK contents{ msg };
        log("MSG_MINER_MANAGER_PROOFOFWORK proof=%", contents.proofOfWork);

        if(validProof(contents.proofOfWork, currentBaseHash))
        {
            log("Proof is valid, Sending to Networker for Propagation.");

            MSG_MANAGER_NETWORKER_PROPAGATENEWBLOCK blockContents;
            blockContents.transactions = postedTransactions;
            blockContents.proofOfWork = contents.proofOfWork;

            Message blockMsg;
            blockContents.compose(blockMsg);
            connFromNetworker.sendMessage(blockMsg);
            
            // Save to Blockchainer module
            // sendMessage(connToBlockchainer.getSocket(), msg);

            log("Starting work on next block.");
        }

        return;
    }
    case MSG_MINER_MANAGER_HASHREQUEST::id:
    {
        log("MSG_MINER_MANAGER_HASHREQUEST");

        MSG_MANAGER_MINER_NEWBASEHASH contents;
        contents.newBaseHash = currentBaseHash;
        Message hashMsg;
        contents.compose(hashMsg);
        connFromMiners.sendMessage(msg.socket, hashMsg);
    }
    }
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
    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };

    log("processTransactionRequestReply recieved % transactions", contents.transactions.size());
    
    for(Transaction& t : contents.transactions)
    {
        postedTransactions.push_back(t);
    }

    u64 newBaseHash = hashVector(postedTransactions);
    if(newBaseHash != currentBaseHash)
    {
        currentBaseHash = newBaseHash;
        log("baseHash has changed, Propagating to Miners.");

        MSG_MANAGER_MINER_NEWBASEHASH contents;
        contents.newBaseHash = currentBaseHash;
        Message hashMsg;
        contents.compose(hashMsg);
        connFromMiners.sendMessage(hashMsg);
    }
    return;
}