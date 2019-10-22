#include <csignal>

#include "Module.h"

std::atomic<bool>* shuttingDownPtr;

void setShutdown(int)
{
    shuttingDownPtr->store(true);
    logger.logInfo("Received shutdown signal. Winding down...");
};

void Module::init(const std::map<str,str>& params)
{
    shuttingDownPtr = &shuttingDown;

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

    for(i32 code : {SIGINT, SIGSTOP, SIGKILL, SIGTSTP, SIGTERM})
    {
        std::signal(code, setShutdown);
    }
}

void Module::run()
{
    while(true)
    {
        if(shuttingDown.load())
        {
            logger.endLogging();
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::vector<EnquedMessage> incomingQueue;
        try
        {
            for(ServerConnection* s : serverConnections)
            {
                s->getIncomingQueue(incomingQueue);
                for(auto& msg : incomingQueue)
                {
                    if(msg.callback.has_value() && msg.message.isReply)
                    {
                        msg.callback.value()(msg.message);
                    }
                    else
                    {
                        processMessage(msg.message);
                    }
                }
                incomingQueue.clear();
            }

            for(ClientConnection* c : clientConnections)
            {
                c->getIncomingQueue(incomingQueue);
                for(auto& msg : incomingQueue)
                {
                    if(msg.callback.has_value() && msg.message.isReply)
                    {
                        msg.callback.value()(msg.message);
                    }
                    else
                    {
                        processMessage(msg.message);
                    }
                }
                incomingQueue.clear();
            }

            u64 now = getCurrentUnixTime();
            for(auto it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it)
            {
                if(it->when <= now)
                {
                    std::invoke(it->task);
                    scheduledTasks.erase(it);
                }
            }
        }
        catch(const std::exception& e)
        {
            logger.logError({
                {"event", "exception"},
                {"description", e.what()}
            });
        }
    }
}

void Module::prepareLogArchive()
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

void Module::sendLogArchive(const Message& msg)
{
    MSG_LOGCOLLECTOR_MODULE_LOGREQUEST incoming{ msg };

    auto archivedLog = std::make_shared<std::ifstream>(logFileName + ".archive");
    if(!archivedLog->is_open())
    {
        throw std::runtime_error("Could not open file!");
    }

    sendNextChunk(msg.reqId, std::move(archivedLog));
}

void Module::sendNextChunk(const u32 reqId, std::shared_ptr<std::ifstream> archivedLog)
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
    const u64 bytesRead = archivedLog->readsome(buffer.data(), 1023);
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

void Module::deleteLogArchive(const Message& msg)
{
    MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK incoming{ msg };

    std::filesystem::remove(logFileName + ".archive");

    registerScheduledTask(LOG_FREQUENCY, [this]()
    {
        prepareLogArchive();
    });
}