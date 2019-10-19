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

    void processManagerBlockRequest(int connSocket, const Message& msg);
    void processManagerBlockRequest_Reply(u32 reqId, const Message& msg);

    void processBlockRequestFromOtherNode(const Message& msg);
    void processBlockRequestFromOtherNode_Reply(int connSocket, u32 reqId, const Message& msg);
};