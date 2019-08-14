#include "Transactioner.h"

Transactioner::Transactioner(const char* iniFileName)
{
    std::cout << "Transactioner Module Starting" << std::endl;

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
    std::cout << "Processing Msg!" << std::endl;
    switch(msg.id)
    {
    case MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ::id:
    {
        MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };
        std::cout << "Manager requesting " << contents.numOfTransactionsRequested << "transactions" << std::endl;

        MSG_A_MANAGER_TRANSACTIONER_TRANSREQ responseContents;

        if(waitingTransactions.empty())
        {
            std::cout << "No waiting transactions" << std::endl;
        }
        else if(waitingTransactions.size() <= contents.numOfTransactionsRequested)
        {
            std::cout << "Less than 100 transactions" << std::endl;
            responseContents.transactions = std::move(waitingTransactions);
            waitingTransactions.clear();
        }
        else
        {
            std::cout << "More than 100 transactions" << std::endl;
            
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

        std::cout << "Sending: " << responseContents.transactions.size() << "transactions to Manager with reqId" << reply.reqId << std::endl;
        connToManager.sendMessage(reply);
        return;
    }
    case MSG_CLIENT_TRANSACTIONER_NEWTRANS::id:
    {
        MSG_CLIENT_TRANSACTIONER_NEWTRANS contents{ msg };

        std::cout << "MSG_CLIENT_TRANSACTIONER_NEWTRANS {" << std::endl;
        std::cout << "timer: " << contents.transaction.time << std::endl;
        std::cout << "sender: " << contents.transaction.sender << std::endl;
        std::cout << "recipiant: " << contents.transaction.recipiant << std::endl;
        std::cout << "amount: " << contents.transaction.amount << std::endl;
        std::cout << "signature: " << contents.transaction.signature << std::endl;
        std::cout << "}";

        // First: Verify Transaction - TODO
        //if(!isTransactionValid(contents.transaction))
        //{
        //    std::cout << "Transaction Signature Invalid" << std::endl;
        //    return;
        //}

        std::cout << "Adding Transaction" << std::endl;
        waitingTransactions.push_back(contents.transaction);

        std::cout << "Total transactions: " << waitingTransactions.size() << std::endl;
        return;
    }
    }
}