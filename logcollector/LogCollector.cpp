#include "LogCollector.h"

LogCollector::LogCollector(const char* iniFileName) : Module("logcollector")
{
    logCollectionEnabled = false;

    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);
    
    logger.logInfo("LogCollector Module Starting");

    logPath = params.at("logPath");
    changeDateIfNecessary();

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
            return;
        }
        default:
        {
            processUnhandledMessage(msg);
            return;
        }
    }
}

void LogCollector::changeDateIfNecessary()
{
    SimpleTime now;
    now.setNowLocal();
    str potentialNewPath = logPath + "/" + now.toString("%Y_%m_%d") + "/";

    if(currentPath != potentialNewPath)
    {
        logger.logInfo({
            {"event", "Log destination folder change"},
            {"currentPath", currentPath},
            {"newPath", potentialNewPath}
        });

        currentPath = std::move(potentialNewPath);
        if(!std::filesystem::exists(currentPath) || !std::filesystem::is_directory(currentPath))
        {
            std::filesystem::create_directory(currentPath);
        }
    }

    registerScheduledTask(ONE_SECOND * 60, [this]()
    {
        changeDateIfNecessary();
    });
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
    MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK incoming(msg);

    if(incoming.log.empty())
    {
        MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK outgoing;
        connFromModules.sendMessage(msg.socket, outgoing.msg(msg.reqId));

        return;
    }
    
    std::ofstream logFile(currentPath + name + ".log", std::ios_base::openmode::_S_app);
    if(!logFile.is_open())
    {
        throw std::runtime_error("Could not open log file");
    }
    logFile << incoming.log;

    MSG_LOGCOLLECTOR_MODULE_LOGREQUEST outgoing;
    connFromModules.sendMessage(
        msg.socket,
        outgoing.msg(msg.reqId),
        [this, name](const Message& msg)
        {
            processLogArchive(msg, name);
        }
    );
}