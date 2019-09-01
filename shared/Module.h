#pragma once

#include <iostream>
#include <thread>
#include <optional>
#include <unordered_map>
#include <functional>
#include <set>
#include <list>

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
    std::vector<ServerConnection*> serverConnnections;
    std::vector<ClientConnection*> clientConnections;
    std::list<ScheduledTask> scheduledTasks;
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
    void registerScheduledTask(u64 millisecondsToWait, F func)
    {
        u64 currentTime = getCurrentUnixTime();
        scheduledTasks.push_back({currentTime + millisecondsToWait, func});
    }
};