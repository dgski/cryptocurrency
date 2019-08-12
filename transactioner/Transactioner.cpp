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
}

void Transactioner::run()
{
    while(true)
    {
        connFromClients.acceptNewConnections();
        
        std::optional<Message> msgFromClient = connFromClients.getMessage();
        if(msgFromClient.has_value())
        {
            processClientMessage(msgFromClient.value());
        }

        std::optional<Message> msgFromManager = connToManager.getMessage();
        if(msgFromManager.has_value())
        {
            processManagerMessage(msgFromManager.value());
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Transactioner::processManagerMessage(Message& msg)
{
    std::cout << "processManagerMessage" << std::endl;
    
    switch(msg.id)
    {
    case MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ::id:
    {
        MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };
        std::cout << "Manager requesting " << contents.numOfTransactionsRequested << "transactions" << std::endl;

        MSG_A_MANAGER_TRANSACTIONER_TRANSREQ responseContents;
        if(waitingTransactions.size() <= contents.numOfTransactionsRequested)
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

        std::cout << "Sending: " << responseContents.transactions.size() << "transactions to Manager with reqId" << reply.reqId << std::endl;
        connToManager.sendMessage(msg);
        return;
    }
    }
}

void Transactioner::processClientMessage(Message& msg)
{
    std::cout << "processClientMessage" << std::endl;
    
    switch(msg.id)
    {
    case MSG_CLIENT_TRANSACTIONER_NEWTRANS::id:
    {
        MSG_CLIENT_TRANSACTIONER_NEWTRANS contents{ msg };

        // First: Verify Transaction - TODO


        waitingTransactions.push_back(contents.transaction);

        std::cout << "MSG_CLIENT_TRANSACTIONER_NEWTRANS {";
        std::cout << "timer: " << contents.transaction.time << std::endl;
        std::cout << "sender: " << contents.transaction.sender << std::endl;
        std::cout << "recipiant: " << contents.transaction.recipiant << std::endl;
        std::cout << "amount: " << contents.transaction.amount << std::endl;
        std::cout << "signature: " << contents.transaction.signature << std::endl;
        std::cout << "}";

        std::cout << "Total transactions: " << waitingTransactions.size() << std::endl;

        return;
    }
    }
}