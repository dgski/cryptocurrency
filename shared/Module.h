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
#include "../shared/Blockchain.h"

class Module
{
    std::vector<ServerConnection*> serverConnnections;
    std::vector<ClientConnection*> clientConnections;
    std::vector<std::function<void()>> repeatedTasks;
public:
    virtual void processMessage(const Message& msg) = 0;
    void run();

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