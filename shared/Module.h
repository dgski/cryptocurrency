#pragma once

#include <iostream>
#include <thread>
#include <optional>
#include <unordered_map>
#include <functional>
#include <set>
#include <list>
#include <variant>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"
#include "../shared/Transaction.h"
#include "../shared/Messages.h"
#include "../shared/Blockchain.h"

struct ScheduledTask
{
    u64 when;
    std::function<void()> task;
};

class Module
{
    std::vector<ServerConnection*> serverConnections;
    std::vector<ClientConnection*> clientConnections;
    std::list<ScheduledTask> scheduledTasks;
public:
    virtual void processMessage(const Message& msg) = 0;
    void run();

    void registerServerConnection(ServerConnection* conn)
    {
        serverConnections.push_back(conn);
    }

    void registerClientConnection(ClientConnection* conn)
    {
        clientConnections.push_back(conn);
    }

    void registerConnections(std::initializer_list<std::variant<ClientConnection*, ServerConnection*>> conns)
    {
        for(auto conn : conns)
        {
            if(std::holds_alternative<ClientConnection*>(conn))
            {
                clientConnections.push_back(std::get<ClientConnection*>(conn));
            }
            else
            {
                serverConnections.push_back(std::get<ServerConnection*>(conn));
            }
        }
    }

    template<typename F>
    void registerScheduledTask(u64 millisecondsToWait, F func)
    {
        u64 currentTime = getCurrentUnixTime();
        scheduledTasks.push_back({currentTime + millisecondsToWait, func});
    }
};