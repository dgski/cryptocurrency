#include "../shared/Module.h"

class LogCollector : public Module
{
    ServerConnection connFromModules;
public:
    LogCollector(const char* iniFileName);
    void processMessage(const Message& msg);
    void processLogReady(const Message& msg);
    void processLogArchive(const Message& msg, const str& name);
};