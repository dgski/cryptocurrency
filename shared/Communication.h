#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <optional>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <thread>

#include "Types.h"
#include "Utils.h"
#include "Logger.h"

enum class ConnectionStatus
{
    Connected,
    Disconnected,
    MessageNotSent
};

class Message
{
public:
    i32 socket;

    u32 id = 0;
    bool isReply = false;
    u32 reqId = 0;
    u64 size = 0;
    std::vector<byte> data;

    void compose_val(u64 val)
    {
        data.resize(data.size() + sizeof(u64));
        memcpy(data.data() + size, &val, sizeof(u64));
        size += sizeof(u64);
    }

    void compose_val(const str& val)
    {
        data.resize(data.size() + val.size() + 1);
        memcpy(data.data() + size, val.c_str(), val.size() + 1);
        size += val.size() + 1;
    }

    void compose_val(i32 val)
    {
        data.resize(data.size() + sizeof(i32));
        memcpy(data.data() + size, &val, sizeof(i32));
        size += sizeof(i32);
    }

    void compose_val(bool val)
    {
        data.resize(data.size() + sizeof(bool));
        memcpy(data.data() + size, &val, sizeof(bool));
        size += sizeof(bool);
    }

    template<typename Col>
    Message& compose_col(const Col& col)
    {
        compose((u64)col.size());
        for(const auto& c : col)
        {
            compose(c);
        }

        return *this;
    }
    
    template<typename T>
    void compose_val(const std::list<T>& col)
    {
        compose_col(col);
    }

    template<typename T>
    void compose_val(const std::vector<T>& col)
    {
        compose_col(col);
    }

    template<typename T>
    void compose_val(const T& t)
    {
        t.compose(*this);
    }

    template<typename Type>
    void compose(Type item)
    {
        compose_val(item);
    }

    template<typename Type, typename... Types>
    void compose(Type& item, Types& ...items)
    {
        compose_val(item);
        compose(items...);
    }

    size_t getFullSize() const
    {
        return sizeof(u32) + sizeof(bool) + sizeof(u32) + sizeof(u64) + data.size();
    }

    void logMsg(const char* direction) const
    {
        logger.logInfo({
            {"event", direction},
            {"socket", socket},
            {"id", id},
            {"isReply", isReply},
            {"reqId", reqId},
            {"size", size}
        });
    }
};

class Parser
{
    const byte* ptr;
public:
    Parser(const Message& msg)
    {
        ptr = msg.data.data();
    }

    void parse_val(u64& val)
    {
        val = *(u64*)ptr;
        ptr += sizeof(u64);
    }

    void parse_val(str& val)
    {
        val.assign((const char*)ptr);
        ptr += val.size() + 1;
    }

    void parse_val(i32& val)
    {
        val = *(i32*)ptr;
        ptr += sizeof(i32);
    }

    void parse_val(bool& val)
    {
        val = *(bool*)ptr;
        ptr += sizeof(bool);
    }

    template<typename Col>
    Parser& parse_col(Col& col)
    {   
        u64 size;
        parse(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& c = col.emplace_back();
            parse(c);
        }

        return *this;
    }
    
    template<typename T>
    void parse_val(std::list<T>& col)
    {
        parse_col(col);
    }

    template<typename T>
    void parse_val(std::vector<T>& col)
    {
        parse_col(col);
    }

    template<typename T>
    void parse_val(T& t)
    {
        t.parse(*this);
    }

    template<typename Type>
    void parse(Type& item)
    {
        parse_val(item);
    }

    template<typename Type, typename... Types>
    void parse(Type& item, Types& ...items)
    {
        parse_val(item);
        parse(items...);
    }
};

std::optional<Message> getFinalMessage(i32 socket);
ConnectionStatus sendFinalMessage(i32 socket, const Message& msg);

using Callback = std::function<void(Message&)>;

struct EnquedMessage
{
    std::optional<i32> socket;
    Message message;
    std::optional<Callback> callback;
};

class Connection
{
protected:
    bool running = false;
    u32 nextReqId = 1;
    std::unordered_map<std::pair<i32, u32>, Callback, pairHash> callbacks;

    std::unique_ptr<std::thread> outgoingThread;
    std::unique_ptr<std::mutex> outgoingMutex;
    std::vector<EnquedMessage> outgoingQueue;

    std::unique_ptr<std::thread> incomingThread;
    std::unique_ptr<std::mutex> incomingMutex;
    std::vector<EnquedMessage> incomingQueue;

    std::atomic<bool> shuttingDown = false;

    u32 getNextReqId()
    {
        const u32 next = nextReqId;
        nextReqId++;

        if(nextReqId == 0)
        {
            nextReqId++;
        }
        return next;
    }
    virtual void runOutgoing() = 0;
    virtual void runIncoming() = 0;
    virtual void closeSockets() = 0;
public:
    virtual void sendMessage(Message& msg, std::optional<Callback> callback = std::nullopt) = 0;
    virtual void sendMessage(Message&& msg, std::optional<Callback> callback = std::nullopt) = 0;

    std::optional<Callback> getCallback(i32 socket, u32 reqId)
    {
        auto it = callbacks.find(std::pair{socket, reqId});
        if(it != callbacks.end())
        {
            auto res = std::move(it->second);
            callbacks.erase(it);
            return res;
        }

        return std::nullopt;
    }

    void getIncomingQueue(std::vector<EnquedMessage>& _incomingQueue)
    {
        std::lock_guard<std::mutex> lock(*incomingMutex);
        std::swap(_incomingQueue, incomingQueue);
    }
    Connection(){}
};

class ServerConnection : public Connection
{
    i32 serverFileDescriptor;

    std::unique_ptr<std::mutex> socketsMutex;
    std::list<i32> sockets;
    sockaddr_in address;

    std::list<EnquedMessage> savedMessages;
protected:
    void closeSockets()
    {
        shutdown(serverFileDescriptor, 2 /* incoming and outgoing */);
        for(i32 socket : sockets)
        {
            shutdown(serverFileDescriptor, 2 /* incoming and outgoing */);
        }
    }

public:
    void init(const IpInfo& ip);
    void acceptNewConnections(const bool wait = false);

    void sendMessage(Message& msg, std::optional<Callback> callback = std::nullopt) override
    {   
        std::lock_guard<std::mutex> lock(*outgoingMutex);
        outgoingQueue.push_back({ std::nullopt, msg, callback });
    }
    void sendMessage(Message&& msg, std::optional<Callback> callback = std::nullopt) override
    {
        sendMessage(msg, callback);
    }
    void sendMessage(int socket, Message& msg, std::optional<Callback> callback = std::nullopt)
    {   
        std::lock_guard<std::mutex> lock(*outgoingMutex);
        outgoingQueue.push_back({socket, msg, callback});
    }
    void sendMessage(int socket, Message&& msg, std::optional<Callback> callback = std::nullopt)
    {
        sendMessage(socket, msg, callback);
    }
    
    void runOutgoing();
    void runIncoming();
    void sendToAll(EnquedMessage& msg);

    ConnectionStatus sendToSingle(EnquedMessage& msg);
    ServerConnection(){}
    ~ServerConnection()
    {
        if(!running)
        {
            return;
        }

        shuttingDown.store(true);
        if(outgoingThread && outgoingThread->joinable())
        {
            outgoingThread->join();
        }
        if(incomingThread && incomingThread->joinable())
        {
            incomingThread->join();
        }
        closeSockets();
    }
};

class ClientConnection : public Connection
{
    i32 socketFileDescriptor;

protected:
    void closeSockets()
    {
        shutdown(socketFileDescriptor, 2 /* incoming and outgoing */);
    }

public:
    void init(const IpInfo& ip);

    int getSocket() const
    {
        return socketFileDescriptor;
    }
    void sendMessage(Message& msg, std::optional<Callback> callback = std::nullopt) override
    {
        std::lock_guard<std::mutex> lock(*outgoingMutex);
        outgoingQueue.push_back({ std::nullopt, msg, callback });
    }
    void sendMessage(Message&& msg, std::optional<Callback> callback = std::nullopt) override
    {
        sendMessage(msg, callback);
    }
    void runOutgoing();
    void runIncoming();

    ClientConnection(){}
    ~ClientConnection()
    {
        if(!running)
        {
            return;
        }

        shuttingDown.store(true);

        if(outgoingThread && outgoingThread->joinable())
        {
            outgoingThread->join();
        }
        if(incomingThread && incomingThread->joinable())
        {
            incomingThread->join();
        }
        closeSockets();
    }
};