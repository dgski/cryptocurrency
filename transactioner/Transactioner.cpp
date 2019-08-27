#include "Transactioner.h"

Transactioner::Transactioner(const char* iniFileName)
{
    log("Transactioner Module Starting");

    const std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    registerClientConnection(&connToManager);

    connFromClients.init(strToIp(params.at("connFromClients")));
    registerServerConnection(&connFromClients);

    registerRepeatedTask([]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
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
    log("MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ numOfTransactionsRequested=%", contents.numOfTransactionsRequested);

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ responseContents;

    if(waitingTransactions.empty())
    {
        log("No waiting transactions");
    }
    else if(waitingTransactions.size() <= contents.numOfTransactionsRequested)
    {
        responseContents.transactions = std::move(waitingTransactions);
        waitingTransactions.clear();
    }
    else
    {            
        std::move(
            end(waitingTransactions) - contents.numOfTransactionsRequested,
            end(waitingTransactions),
            std::back_inserter(responseContents.transactions)
        );

        waitingTransactions.erase(
            end(waitingTransactions) - contents.numOfTransactionsRequested,
            end(waitingTransactions)
        );
    }
    
    Message reply;
    reply.reqId = msg.reqId;
    responseContents.compose(reply);

    log("Sending % transactions to Manager", responseContents.transactions.size());
    connToManager.sendMessage(reply);
}

void Transactioner::processAddNewTransaction(const Message& msg)
{
    MSG_CLIENT_TRANSACTIONER_NEWTRANS contents{ msg };
    log("MSG_CLIENT_TRANSACTIONER_NEWTRANS");

    if(!isTransactionSignatureValid(contents.transaction))
    {
        std::cout << "Transaction Signature Invalid" << std::endl;
        return;
    }

    // Second: Check if account has enough funds

    log("Adding transaction");
    waitingTransactions.push_back(contents.transaction);

    log("Total waiting transactions=%", waitingTransactions.size());
}