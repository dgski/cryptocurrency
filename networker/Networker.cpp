#include "Networker.h"

Networker::Networker(const char* iniFileName)
{
    log("Networker Module Starting");

    const std::map<str,str> params = getInitParameters(iniFileName);

    connToManager.init(strToIp(params.at("connToManager")));
    connFromOtherNodes.init(strToIp(params.at("connFromOtherNodes")));
    registerConnections({&connToManager, &connFromOtherNodes});

    const auto connStrings = splitStr(params.at("connsToOtherNodes"));
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
        case MSG_MANAGER_NETWORKER_CHAINREQUEST::id:
        {   
            processManagerChainRequest(msg);
            return;
        }
        case MSG_NETWORKER_NETWORKER_CHAINREQUEST::id:
        {
            processChainRequestFromOtherNode(msg);
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
    MSG_MANAGER_NETWORKER_NEWBLOCK incoming{ msg };

    MSG_NETWORKER_NETWORKER_NEWBLOCK outgoing;
    outgoing.block = std::move(incoming.block);

    auto msgToPropagate = outgoing.msg();
    for(Connection& conn : connsToOtherNodes)
    {
        conn.sendMessage(msgToPropagate);
    }
}

void Networker::processNewBlockFromOtherNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_NEWBLOCK incoming{ msg };   

    if(!incoming.block.isValid())
    {
        log("Block is invalid");
        return;
    }

    MSG_NETWORKER_MANAGER_NEWBLOCK outgoing;
    outgoing.block = std::move(incoming.block);
    outgoing.connId = msg.socket;

    connToManager.sendMessage(outgoing.msg());
}

void Networker::processRegisterNewNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_REGISTERME incoming{ msg };

    ClientConnection& newConn = connsToOtherNodes.emplace_back();
    newConn.init(strToIp(incoming.connStr));
    registerClientConnection(&newConn);
}

void Networker::processManagerChainRequest(const Message& msg)
{
    MSG_MANAGER_NETWORKER_CHAINREQUEST incoming{ msg };

    MSG_NETWORKER_NETWORKER_CHAINREQUEST outgoing;
    outgoing.maxId = incoming.maxId;

    connFromOtherNodes.sendMessage(
        incoming.connId,
        outgoing.msg(),
        [this, reqId = msg.reqId](const Message& msg)
        {
        processManagerChainRequest_Reply(reqId, msg);
        }
    );
}

void Networker::processManagerChainRequest_Reply(u32 reqId, const Message& msg)
{
    MSG_NETWORKER_NETWORKER_CHAIN incoming{ msg };

    MSG_NETWORKER_MANAGER_CHAIN outgoing;
    outgoing.chain = std::move(incoming.chain);

    connToManager.sendMessage(outgoing.msg(msg.reqId));
}

void Networker::processChainRequestFromOtherNode(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_CHAINREQUEST incoming{ msg };

    MSG_NETWORKER_MANAGER_CHAINREQUEST outgoing;
    outgoing.maxId = incoming.maxId;

    connToManager.sendMessage(outgoing.msg(), [this, reqId = msg.reqId](const Message& msg)
    {
        processChainRequestFromOtherNode_Reply(reqId, msg);
    });
}

void Networker::processChainRequestFromOtherNode_Reply(u32 reqId, const Message& msg)
{
    MSG_MANAGER_NETWORKER_CHAIN incoming{ msg };

    MSG_NETWORKER_NETWORKER_CHAIN outgoing;
    outgoing.chain = std::move(incoming.chain);

    connFromOtherNodes.sendMessage(outgoing.msg(msg.reqId));    
}