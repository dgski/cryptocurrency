#include "LogCollector.h"

LogCollector::LogCollector(const char* iniFileName)
{
    logCollectionEnabled = false;

    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);
    
    logger.logInfo("LogCollector Module Starting");

    connFromModules.init(strToIp(params.at("connFromModules")));
    registerServerConnection(&connFromModules);
}

void LogCollector::processMessage(const Message& msg)
{
    switch(msg.id)
    {
        case MSG_MODULE_LOGCOLLECTOR_LOGREADY::id:
        {
            processLogReady(msg);
        }
        default:
        {
            processUnhandledMessage(msg);
            return;
        }
    }
}

void LogCollector::processLogReady(const Message& msg)
{
    MSG_MODULE_LOGCOLLECTOR_LOGREADY incoming{ msg };

    MSG_LOGCOLLECTOR_MODULE_LOGREQUEST outgoing;
    connFromModules.sendMessage(
        msg.socket,
        outgoing.msg(msg.reqId),
        [this, name = incoming.name](const Message& msg)
        {
            processLogArchive(msg, name);
        }
    );
}

void LogCollector::processLogArchive(const Message& msg, const str& name)
{
    MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE incoming(msg);
    
    std::ofstream logFile(name + ".log");
    if(!logFile.is_open())
    {
        throw std::runtime_error("Could not open log file");
    }
    logFile << incoming.log;

    MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK outgoing;
    connFromModules.sendMessage(msg.socket, outgoing.msg(msg.reqId));
}