#include "Transactioner.h"

Transactioner::Transactioner(const char* iniFileName) : Module("transactioner")
{
    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);

    logger.logInfo("Transactioner Module Starting");

    connToManager.init(strToIp(params.at("connToManager")));
    connFromClients.init(strToIp(params.at("connFromClients")));
    registerConnections({&connToManager, &connFromClients});
}

void Transactioner::processMessage(const Message& msg)
{
    switch(msg.id)
    {
        case MSG_MANAGER_TRANSACTIONER_TRANSREQ::id:
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
            processUnhandledMessage(msg);
            return;
        }
    }
}

void Transactioner::processRequestForTransactions(const Message& msg)
{
    MSG_MANAGER_TRANSACTIONER_TRANSREQ incoming{ msg };

    MSG_MANAGER_TRANSACTIONER_TRANSREQ_REPLY outgoing;

    if(waitingTransactions.empty())
    {
        logger.logInfo("No waiting transactions");
    }
    else if(waitingTransactions.size() <= incoming.numOfTransReq)
    {
        outgoing.transactions = std::move(waitingTransactions);
        waitingTransactions.clear();
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
    
    logger.logInfo({
        {"event", "Sending transactions to Manager"},
        {"outgoing.transactions.size()", (u64)outgoing.transactions.size()}
    });
    connToManager.sendMessage(outgoing.msg(msg.reqId));
}

void Transactioner::processAddNewTransaction(const Message& msg)
{
    MSG_CLIENT_TRANSACTIONER_NEWTRANS incoming{ msg };

    if(!incoming.transaction.isSignatureValid())
    {
        logger.logWarning("Signature Invalid. Rejecting Transaction.");
        return;
    }

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET outgoing;
    outgoing.publicWalletKey = incoming.transaction.sender;
    connToManager.sendMessage(
        outgoing.msg(),
        [this, transaction = incoming.transaction](const Message& msg) mutable
        {
            processAddNewTransaction_Finalize(msg, transaction);
        }
    );
}

void Transactioner::processAddNewTransaction_Finalize(
    const Message& msg,
    Transaction& transaction)
{
    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY incoming{ msg };

    if(transaction.amount >= incoming.amount)
    {
        logger.logInfo({
            {"event", "Not enough funds in wallet. Rejecting Transaction."},
            {"transaction.amount", transaction.amount},
            {"wallet.amount", incoming.amount}
        });
        return;
    }
    
    logger.logInfo("Adding transaction");
    waitingTransactions.push_back(std::move(transaction));

    logger.logInfo({
        {"event", "Total waiting transactions report"},
        {"waitingTransactions.size()", (u64)waitingTransactions.size()}
    });
}