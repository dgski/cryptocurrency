#include "LogCollector.h"

LogCollector::LogCollector(const char* iniFileName) : Module()
{
    logCollectionEnabled = false;

    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);
    
    logger.logInfo("LogCollector Module Starting");

    connFromModules.init(strToIp(params.at("connFromModules")));
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

    // TODO log info here

    MSG_LOGCOLLECTOR_MODULE_LOGREQUEST outgoing;
    connFromModules.sendMessage(
        msg.socket,
        outgoing.msg(msg.reqId),
        [this](const Message& msg)
        {
            processLogArchive(msg);
        }
    );
}

void LogCollector::processLogArchive(const Message& msg)
{
    MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE incoming(msg);

    // TODO save file locally here

    MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK outgoing;
    connFromModules.sendMessage(msg.socket, outgoing.msg(msg.reqId));
}