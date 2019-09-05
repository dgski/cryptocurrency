#include "Transactioner.h"

Transactioner::Transactioner(const char* iniFileName)
{
    log("Transactioner Module Starting");

    const std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    registerClientConnection(&connToManager);

    connFromClients.init(strToIp(params.at("connFromClients")));
    registerServerConnection(&connFromClients);
}

void Transactioner::processMessage(const Message& msg)
{
    switch(msg.id)
    {
        case MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ::id:
        {
            processRequestForTransactions(msg);
            return;
        }
        case MSG_CLIENT_TRANSACTIONER_NEWTRANS::id:
        {
            processAddNewTransaction(msg);
            return;
        }
        default:
        {
            log("Unhandled MSG id=%", msg.id);
            return;
        }
    }
}

void Transactioner::processRequestForTransactions(const Message& msg)
{
    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ reply;

    if(waitingTransactions.empty())
    {
        log("No waiting transactions");
    }
    else if(waitingTransactions.size() <= contents.numOfTransactionsRequested)
    {
        reply.transactions = std::move(waitingTransactions);
        waitingTransactions.clear();
    }
    else
    {            
        std::move(
            std::begin(waitingTransactions),
            std::begin(waitingTransactions) + contents.numOfTransactionsRequested,
            std::back_inserter(reply.transactions)
        );

        waitingTransactions.erase(
            std::begin(waitingTransactions),
            std::begin(waitingTransactions) + contents.numOfTransactionsRequested
        );
    }
    
    log("Sending % transactions to Manager", reply.transactions.size());
    connToManager.sendMessage(reply.msg());
}

void Transactioner::processAddNewTransaction(const Message& msg)
{
    MSG_CLIENT_TRANSACTIONER_NEWTRANS contents{ msg };

    if(!isTransactionSignatureValid(contents.transaction))
    {
        log("Transaction Signature Invalid.");
        return;
    }

    // Second: Check if account has enough funds

    log("Adding transaction");
    waitingTransactions.push_back(contents.transaction);
    
    log("Total waiting transactions=%", waitingTransactions.size());
}