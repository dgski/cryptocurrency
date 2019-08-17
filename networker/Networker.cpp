#include "Networker.h"

Networker::Networker(const char* iniFileName)
{
    log("Networker Module Starting");

    std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(
        params["connToManagerIP"].c_str(),
        atoi(params["connToManagerPORT"].c_str())
    );

    registerClientConnection(&connToManager);
}

void Networker::processMessage(Message& msg)
{
    switch(msg.id)
    {
    case MSG_MANAGER_NETWORKER_PROPAGATENEWBLOCK::id:
    {
        MSG_MANAGER_NETWORKER_PROPAGATENEWBLOCK contents{ msg };
        log(
            "Received new Block from Manager: numOfTrans=%, proofOfWork=%",
            contents.transactions.size(),
            contents.proofOfWork
        );

        MSG_NETWORKER_NETWORKER_NEWBLOCK contentsToPropagate;
        contentsToPropagate.transactions = move(contents.transactions);
        contentsToPropagate.proofOfWork = contents.proofOfWork;
        // TODO: Send to other networker modules
        return;
    }
    case MSG_NETWORKER_NETWORKER_NEWBLOCK::id:
    {
        MSG_NETWORKER_NETWORKER_NEWBLOCK contents{ msg };
        log(
            "Received new Block from external Node: numOfTrans=%, proofOfWork=%",
            contents.transactions.size(),
            contents.proofOfWork
        );

        // Validate
        // TODO

        // Forward To Manager
        // TODO
    }
    }
}