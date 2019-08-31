#include "../shared/Module.h"

class Networker : public Module
{
    ClientConnection connToManager;
    std::vector<ClientConnection> connsToOtherNodes;
    ServerConnection connFromOtherNodes;
public:
    Networker(const char* iniFileName);
    void processMessage(const Message& msg);
    void processNewBlockFromManager(const Message& msg);
    void processNewBlockFromOtherNode(const Message& msg);
    void processRegisterNewNode(const Message& msg);
};