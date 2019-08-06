#include <list>

#include "../shared/Module.h"

class Transactioner : Module
{
    ClientConnection connToManager;
    ServerConnection connFromClients;

    std::vector<Transaction> waitingTransactions;

public:
    Transactioner(const char* iniFileName)
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
    void run()
    {
        while(true)
        {
            // Get Connections From Clients
            connFromClients.acceptNewConnections();
            for(int socket : connFromClients.sockets)
            {
                std::optional<Message> msg = getMessage(socket);
                if(msg.has_value())
                {
                    processClientMessage(msg.value());
                }
            }

            std::optional<Message> msg = getMessage(connToManager.getSocket());
            if(msg.has_value())
            {
                processMessage(msg.value());
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void processMessage(Message& msg)
    {
        switch(msg.id)
        {
        case MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ::id:
        {
            MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents;
            Parser parser(msg);
            contents.parse(parser);
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
            
            Message msg;
            responseContents.compose(msg);
            sendMessage(connToManager.getSocket(), msg);
        }
        }
    }

    void processClientMessage(Message& msg)
    {
        switch(msg.id)
        {
        case MSG_CLIENT_TRANSACTIONER_NEWTRANS::id:
        {
            MSG_CLIENT_TRANSACTIONER_NEWTRANS contents;
            Parser parser(msg);
            contents.parse(parser);

            // Verify Transaction
            waitingTransactions.push_back(contents.transaction);
            std::cout << "Added 1 New Transaction to Waiting List with id: " << contents.transaction.id << std::endl;
            std::cout << "Total transaction: " << waitingTransactions.size() << std::endl;
        }
        }
    }
};