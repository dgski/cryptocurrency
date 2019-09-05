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
    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ incoming{ msg };

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ outgoing;

    if(waitingTransactions.empty())
    {
        log("No waiting transactions");
    }
    else if(waitingTransactions.size() <= incoming.numOfTransactionsRequested)
    {
        outgoing.transactions = std::move(waitingTransactions);
        waitingTransactions.clear();
    }
    else
    {            
        std::move(
            std::begin(waitingTransactions),
            std::begin(waitingTransactions) + incoming.numOfTransactionsRequested,
            std::back_inserter(outgoing.transactions)
        );

        waitingTransactions.erase(
            std::begin(waitingTransactions),
            std::begin(waitingTransactions) + incoming.numOfTransactionsRequested
        );
    }
    
    log("Sending % transactions to Manager", outgoing.transactions.size());
    connToManager.sendMessage(outgoing.msg());
}

void Transactioner::processAddNewTransaction(const Message& msg)
{
    MSG_CLIENT_TRANSACTIONER_NEWTRANS incoming{ msg };

    if(!isTransactionSignatureValid(incoming.transaction))
    {
        log("Transaction Signature Invalid.");
        return;
    }

    // Second: Check if account has enough funds

    log("Adding transaction");
    waitingTransactions.push_back(incoming.transaction);
    
    log("Total waiting transactions=%", waitingTransactions.size());
}