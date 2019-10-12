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
#include <filesystem>
#include <array>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"
#include "../shared/Transaction.h"
#include "../shared/Messages.h"
#include "../shared/Blockchain.h"
#include "../shared/Logger.h"

constexpr u32 LOG_FREQUENCY = 30 * ONE_SECOND;

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

    str logFileName;
    std::ofstream logFile;

    ClientConnection connToLogCollector;
protected:
    const str moduleName;
public:
    bool logCollectionEnabled = true;
    
    Module(str _moduleName)
    : moduleName(std::move(_moduleName))
    {}

    void init(const std::map<str,str>& params)
    {
        logFileName = params.at("logFileName");

        logFile.open(logFileName.c_str(), std::ios_base::openmode::_S_app);
        if(!logFile.is_open())
        {
            throw std::runtime_error("Could not open file!");
        }
        logger.addOutputStream(&std::cout);
        logger.addOutputStream(&logFile);
        logger.run();

        if(logCollectionEnabled)
        {
            connToLogCollector.init(strToIp(params.at("connToLogCollector")));
            registerClientConnection(&connToLogCollector);
            registerScheduledTask(LOG_FREQUENCY, [this]()
            {
                prepareLogArchive();
            });
        }
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

    void prepareLogArchive()
    {
        logger.logInfo("Preparing log Archive");
        
        logger.endLogging();
        logFile.close();

        std::filesystem::rename(logFileName, logFileName + ".archive");
        
        logFile.open(logFileName.c_str(), std::ios_base::openmode::_S_app);
        if(!logFile.is_open())
        {
            throw std::runtime_error("Could not open file!");
        }

        logger.addOutputStream(&std::cout);
        logger.addOutputStream(&logFile);
        logger.run();

        MSG_MODULE_LOGCOLLECTOR_LOGREADY outgoing;
        outgoing.name = moduleName;

        connToLogCollector.sendMessage(outgoing.msg(), [this](const Message& msg)
        {
            sendLogArchive(msg);
        });
    }

    void sendLogArchive(const Message& msg)
    {
        MSG_LOGCOLLECTOR_MODULE_LOGREQUEST incoming{ msg };

        auto archivedLog = std::make_shared<std::ifstream>(logFileName + ".archive");
        if(!archivedLog->is_open())
        {
            throw std::runtime_error("Could not open file!");
        }

        sendNextChunk(msg.reqId, std::move(archivedLog));
    }

    void sendNextChunk(u32 reqId, std::shared_ptr<std::ifstream> archivedLog)
    {        
        MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK outgoing;

        if(archivedLog->peek() == EOF) // Send empty
        {
            connToLogCollector.sendMessage(
                outgoing.msg(reqId),[this](const Message& msg)
                {
                    deleteLogArchive(msg);
                });

            return;
        }

        std::array<char, 1024> buffer;
        u64 bytesRead = archivedLog->readsome(buffer.data(), 1023);
        buffer[bytesRead] = '\0';

        outgoing.log.reserve(bytesRead);
        for(char c : buffer)
        {
            if(c == '\0')
            {
                break;
            }
            outgoing.log.push_back(c);
        }

        connToLogCollector.sendMessage(
            outgoing.msg(reqId),[this, archivedLog = std::move(archivedLog)](const Message& msg)
            {
                MSG_LOGCOLLECTOR_MODULE_LOGREQUEST incoming{ msg };
                sendNextChunk(msg.reqId, archivedLog);
            });
    }

    void deleteLogArchive(const Message& msg)
    {
        MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK incoming{ msg };

        std::filesystem::remove(logFileName + ".archive");

        registerScheduledTask(LOG_FREQUENCY, [this]()
        {
            prepareLogArchive();
        });
    }
};