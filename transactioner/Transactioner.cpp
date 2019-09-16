#include "Transactioner.h"

Transactioner::Transactioner(const char* iniFileName)
{
    log("Transactioner Module Starting");

    const std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    connFromClients.init(strToIp(params.at("connFromClients")));
    registerConnections({&connToManager, &connFromClients});
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
    else if(waitingTransactions.size() <= incoming.numOfTransReq)
    {
        outgoing.transactions.splice(
            std::cbegin(outgoing.transactions),
            waitingTransactions
        );
    }
    else
    {
        outgoing.transactions.splice(
            std::cbegin(outgoing.transactions),
            waitingTransactions,
            std::cbegin(waitingTransactions),
            std::next(std::cbegin(waitingTransactions), incoming.numOfTransReq)
        );
    }
    
    log("Sending % transactions to Manager", outgoing.transactions.size());
    connToManager.sendMessage(outgoing.msg(msg.reqId));
}

void Transactioner::processAddNewTransaction(const Message& msg)
{
    MSG_CLIENT_TRANSACTIONER_NEWTRANS incoming{ msg };

    if(!incoming.transaction.isSignatureValid())
    {
        log("Signature Invalid. Rejecting Transaction.");
        return;
    }

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET outgoing;
    outgoing.publicWalletKey = incoming.transaction.sender;
    connToManager.sendMessage(outgoing.msg(), [this, transaction = incoming.transaction](const Message& msg)
    {
        processAddNewTransaction_Finalize(msg, transaction);
    });
}

void Transactioner::processAddNewTransaction_Finalize(const Message& msg, const Transaction& transaction)
{
    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY incoming{ msg };

    if(transaction.amount > incoming.amount)
    {
        log("Not enough funds in wallet. Rejecting Transaction.");
        return;
    }
    
    log("Adding transaction");
    waitingTransactions.push_back(transaction);
    log("Total waiting transactions=%", waitingTransactions.size());
}