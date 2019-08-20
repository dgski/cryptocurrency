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
    case MSG_MANAGER_NETWORKER_NEWBLOCK::id:
    {
        MSG_MANAGER_NETWORKER_NEWBLOCK contents{ msg };
        log(
            "Received new Block from Manager: numOfTrans=%, proofOfWork=%",
            contents.block.transactions.size(),
            contents.block.proofOfWork
        );

        MSG_NETWORKER_NETWORKER_NEWBLOCK contentsToPropagate;
        contentsToPropagate.block.transactions = move(contents.block.transactions);
        contentsToPropagate.block.proofOfWork = contents.block.proofOfWork;
        // TODO: Send to other networker modules
        return;
    }
    case MSG_NETWORKER_NETWORKER_NEWBLOCK::id:
    {
        MSG_NETWORKER_NETWORKER_NEWBLOCK contents{ msg };
        log(
            "Received new Block from external Node: numOfTrans=%, proofOfWork=%",
            contents.block.transactions.size(),
            contents.block.proofOfWork
        );

        // Validate
        // TODO

        // Forward To Manager
        // TODO
    }
    }
}