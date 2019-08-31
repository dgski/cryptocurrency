#include "Networker.h"

Networker::Networker(const char* iniFileName)
{
    log("Networker Module Starting");

    std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    registerClientConnection(&connToManager);

    connFromOtherNodes.init(strToIp(params.at("connFromOtherNodes")));
    registerServerConnection(&connFromOtherNodes);

    auto connStrings = splitStr(params.at("connsToOtherNodes"));
    for(const str& connString : connStrings)
    {
        auto& newConn = connsToOtherNodes.emplace_back();
        newConn.init(strToIp(connString));
        registerClientConnection(&newConn);

        MSG_NETWORKER_NETWORKER_REGISTERME contents;
        contents.connStr = params.at("connFromOtherNodes");
        Message msg;
        contents.compose(msg);
        newConn.sendMessage(msg); // TODO Safety
    }
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
        case MSG_NETWORKER_NETWORKER_REGISTERME::id:
        {
            processRegisterNewNode(msg);
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
    for(Connection& conn : connsToOtherNodes)
    {
        conn.sendMessage(msgToPropagate);
    }
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

    MSG_NETWORKER_MANAGER_NEWBLOCK contentsToManager;
    contentsToManager.block = std::move(contents.block);
    contentsToManager.connId = msg.socket;
    
    Message msgToManager;
    contentsToManager.compose(msgToManager);

    connToManager.sendMessage(msgToManager);
}

void Networker::processRegisterNewNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_REGISTERME contents{ msg };
    log("Registering New Node at: %", contents.connStr);

    auto& newConn = connsToOtherNodes.emplace_back();
    newConn.init(strToIp(contents.connStr));
    registerClientConnection(&newConn);
}