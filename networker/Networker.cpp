#include "Networker.h"

Networker::Networker(const char* iniFileName)
{
    log("Networker Module Starting");

    std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    registerClientConnection(&connToManager);

    connFromOtherNodes.init(strToIp(params.at("connFromOtherNodes")));
    registerServerConnection(&connFromOtherNodes);

    //registerClientConnection(&connToOtherNodes);
}

void Networker::processMessage(const Message& msg)
{
    switch(msg.id)
    {
        case MSG_MANAGER_NETWORKER_NEWBLOCK::id:
        {
            processNewBlockFromManager(msg);
            return;
        }
        case MSG_NETWORKER_NETWORKER_NEWBLOCK::id:
        {
            processNewBlockFromOtherNode(msg);
            return;
        }
        default:
        {
            log("Unhandled MSG id=%", msg.id);
            return;
        }
    }
}

void Networker::processNewBlockFromManager(const Message& msg)
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

    Message msgToPropagate;
    contentsToPropagate.compose(msgToPropagate);
    connFromOtherNodes.sendMessage(msgToPropagate);
}

void Networker::processNewBlockFromOtherNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_NEWBLOCK contents{ msg };   
    log(
        "Received new Block from external Node: numOfTrans=%, proofOfWork=%",
        contents.block.transactions.size(),
        contents.block.proofOfWork
    );

    if(!contents.block.isValid())
    {
        log("Block is invalid");
        return;
    }

    // Send to Manager
}