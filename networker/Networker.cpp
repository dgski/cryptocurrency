#include "Networker.h"

Networker::Networker(const char* iniFileName) : Module("networker")
{
    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);

    logger.logInfo("Networker Module Starting");

    connToManager.init(strToIp(params.at("connToManager")));
    connFromOtherNodes.init(strToIp(params.at("connFromOtherNodes")));
    registerConnections({&connToManager, &connFromOtherNodes});

    const auto connStrings = splitStr(params.at("connsToOtherNodes"));
    logger.logInfo({
        {"event", "Registering External Nodes"},
        {"count", (u64)connStrings.size()}
    });
    for(const str& connString : connStrings)
    {
        auto& newConn = connsToOtherNodes.emplace_back();
        newConn.init(strToIp(connString));
        registerClientConnection(&newConn);

        MSG_NETWORKER_NETWORKER_REGISTERME outgoing;
        outgoing.connStr = params.at("connFromOtherNodes");
        newConn.sendMessage(outgoing.msg());
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
        case MSG_NETWORKER_NETWORKER_BLOCKREQUEST::id:
        {
            processBlockRequestFromOtherNode(msg);
            return;
        }
        default:
        {
            processUnhandledMessage(msg);
            return;
        }
    }
}

void Networker::processNewBlockFromManager(const Message& msg)
{
    MSG_MANAGER_NETWORKER_NEWBLOCK incoming{ msg };

    MSG_NETWORKER_NETWORKER_NEWBLOCK outgoing;
    outgoing.block = std::move(incoming.block);

    auto msgToPropagate = outgoing.msg();
    for(Connection& conn : connsToOtherNodes)
    {
        conn.sendMessage(msgToPropagate);
    }
}

void Networker::processRegisterNewNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_REGISTERME incoming{ msg };

    ClientConnection& newConn = connsToOtherNodes.emplace_back();
    newConn.init(strToIp(incoming.connStr));
    registerClientConnection(&newConn);
}

void Networker::processNewBlockFromOtherNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_NEWBLOCK incoming{ msg };   

    if(!incoming.block.isValid())
    {
        logger.logWarning("Block is invalid");
        return;
    }

    MSG_NETWORKER_MANAGER_NEWBLOCK outgoing;
    outgoing.block = std::move(incoming.block);

    connToManager.sendMessage(outgoing.msg(),[this, connSocket = msg.socket](const Message& msg)
    {
        processManagerBlockRequest(connSocket, msg);
    });
}

void Networker::processManagerBlockRequest(int connSocket, const Message& msg)
{
    MSG_MANAGER_NETWORKER_BLOCKREQUEST incoming{ msg };

    if(incoming.voidRequest)
    {
        return;
    }

    MSG_NETWORKER_NETWORKER_BLOCKREQUEST outgoing;
    outgoing.blockId = incoming.blockId;

    connFromOtherNodes.sendMessage(
        connSocket,
        outgoing.msg(),
        [this, reqId = msg.reqId](const Message& msg)
        {
            processManagerBlockRequest_Reply(reqId, msg);
        }
    );
}

void Networker::processManagerBlockRequest_Reply(u32 reqId, const Message& msg)
{
    MSG_NETWORKER_NETWORKER_BLOCK incoming{ msg };

    MSG_NETWORKER_MANAGER_BLOCK outgoing;
    outgoing.block = std::move(incoming.block);

    connToManager.sendMessage(outgoing.msg(reqId),[this, connSocket = msg.socket](const Message& msg)
    {
        processManagerBlockRequest(connSocket, msg);
    });
}

void Networker::processBlockRequestFromOtherNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_BLOCKREQUEST incoming{ msg };

    MSG_NETWORKER_MANAGER_BLOCKREQUEST outgoing;
    outgoing.blockId = incoming.blockId;

    i32 connSocket = msg.socket;
    logger.logInfo({{"connSocket", connSocket}});
    connToManager.sendMessage(outgoing.msg(), [this, connSocket, reqId = msg.reqId](const Message& msg)
    {
        processBlockRequestFromOtherNode_Reply(connSocket, reqId, msg);
    });
}

void Networker::processBlockRequestFromOtherNode_Reply(int connSocket, u32 reqId, const Message& msg)
{
    MSG_MANAGER_NETWORKER_BLOCK incoming{ msg };

    MSG_NETWORKER_NETWORKER_BLOCK outgoing;
    outgoing.block = std::move(incoming.block);

    connFromOtherNodes.sendMessage(connSocket, outgoing.msg(reqId));    
}