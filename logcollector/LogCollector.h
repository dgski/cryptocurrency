#include "../shared/Module.h"

class LogCollector : public Module
{
    ServerConnection connFromModules;
    str logPath;
    str currentPath;
public:
    LogCollector(const char* iniFileName);
    void processMessage(const Message& msg);
    void changeDateIfNecessary();
    void processLogReady(const Message& msg);
    void processLogArchive(const Message& msg, const str& name);
};