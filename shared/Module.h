#pragma once

#include <iostream>
#include <thread>
#include <optional>
#include <unordered_map>
#include <functional>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"
#include "../shared/Transaction.h"
#include "../shared/Messages.h"

class Module
{
    std::vector<ServerConnection*> serverConnnections;
    std::vector<ClientConnection*> clientConnections;
    std::vector<std::function<void()>> repeatedTasks;
public:
    virtual void run() = 0;

    virtual void processMessage(Message& msg) = 0;
    void run2()
    {
        while(true)
        {
            std::cout << "Cycle" << std::endl;
            
            for(ServerConnection* s : serverConnnections)
            {
                s->acceptNewConnections();
                std::optional<Message> msg = s->getMessage();
                if(msg.has_value())
                {
                    processMessage(msg.value());
                }
            }

            for(ClientConnection* c : clientConnections)
            {
                std::optional<Message> msg = c->getMessage();
                if(msg.has_value())
                {
                    processMessage(msg.value());
                }
            }

            for(auto& task : repeatedTasks)
            {
                task();
            }

            std::this_thread::sleep_for (std::chrono::seconds(1));
        }
    }

    void registerServerConnection(ServerConnection* conn)
    {
        serverConnnections.push_back(conn);
    }

    void registerClientConnection(ClientConnection* conn)
    {
        clientConnections.push_back(conn);
    }

    template<typename F>
    void registerRepeatedTask(F func)
    {
        repeatedTasks.push_back(func);
    }
};