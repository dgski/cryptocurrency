#include "Transactioner.h"

Transactioner::Transactioner(const char* iniFileName)
{
    log("Transactioner Module Starting");


    std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(
        params["connToManagerIP"].c_str(),
        atoi(params["connToManagerPORT"].c_str())
    );

    connFromClients.init(
        params["connFromClientsIP"].c_str(),
        atoi(params["connFromClientsPORT"].c_str())
    );

    registerRepeatedTask([]()
    {
        std::this_thread::sleep_for (std::chrono::seconds(1));
    });

    registerClientConnection(&connToManager);
    registerServerConnection(&connFromClients);
}

void Transactioner::processMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ::id:
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

        log("Sending % transactions to Manager");
        connToManager.sendMessage(reply);
        return;
    }
    case MSG_CLIENT_TRANSACTIONER_NEWTRANS::id:
    {
        MSG_CLIENT_TRANSACTIONER_NEWTRANS contents{ msg };
        log("MSG_CLIENT_TRANSACTIONER_NEWTRANS");

        /*
        // First: Verify Transaction
        if(!isTransactionValid(contents.transaction))
        {
            std::cout << "Transaction Signature Invalid" << std::endl;
            return;
        }
        */

        log("Adding transaction");
        waitingTransactions.push_back(contents.transaction);

        log("Total waiting transactions=%", waitingTransactions.size());
        return;
    }
    }
}