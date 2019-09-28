#pragma once

#include <iostream>
#include <thread>
#include <optional>
#include <unordered_map>
#include <functional>
#include <set>
#include <list>
#include <variant>
#include <fstream>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"
#include "../shared/Transaction.h"
#include "../shared/Messages.h"
#include "../shared/Blockchain.h"
#include "../shared/Logger.h"

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
    std::ofstream logFile;
public:
    Module()
    {
        
    }

    void initLogger(const char* logFileName)
    {
        logFile.open(logFileName, std::ios_base::openmode::_S_app);
        if(!logFile.is_open())
        {
            throw std::runtime_error("Could not open file!");
        }
        logger.addOutputStream(&std::cout);
        logger.addOutputStream(&logFile);
        logger.run();
    }

    virtual void processMessage(const Message& msg) = 0;

    void processUnhandledMessage(const Message& msg)
    {
        logger.logInfo({
            {"event", "Unhandled MSG"},
            {"msgId", msg.id}
        });
    }

    void run();

    void registerServerConnection(ServerConnection* conn)
    {
        serverConnections.push_back(conn);
    }

    void registerClientConnection(ClientConnection* conn)
    {
        clientConnections.push_back(conn);
    }

    void registerConnections(
        std::initializer_list<std::variant<ClientConnection*, ServerConnection*>> conns)
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