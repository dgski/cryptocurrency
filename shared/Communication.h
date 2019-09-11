#pragma once

#include <iostream>
#include <vector>
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

#include "Types.h"
#include "Utils.h"

class Message
{
public:
    int socket;

    u32 id = 0;
    u32 reqId = 0;
    u64 size = 0;
    std::vector<byte> data;

    Message& compose_u64(u64 val)
    {
        data.resize(data.size() + sizeof(u64));
        memcpy(data.data() + size, &val, sizeof(u64));
        size += sizeof(u64);

        return *this;
    }

    Message& compose_str(const str& val)
    {
        data.resize(data.size() + val.size() + 1);
        memcpy(data.data() + size, val.c_str(), val.size() + 1);
        size += val.size() + 1;

        return *this;
    }

    Message& compose_i32(i32 val)
    {
        data.resize(data.size() + sizeof(i32));
        memcpy(data.data() + size, &val, sizeof(i32));
        size += sizeof(i32);

        return *this;
    }

    template<typename Col>
    Message& compose_col(const Col& col)
    {
        compose_u64((u64)col.size());
        for(const auto& c : col)
        {
            c.compose(*this);
        }

        return *this;
    }

    void compose_val(u64 val)
    {
        compose_u64(val);
    }

    void compose_val(const str& val)
    {
        compose_str(val);
    }

    void compose_val(i32 val)
    {
        compose_i32(val);
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
        return sizeof(u32) + sizeof(u32) + sizeof(u64) + data.size();
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

    Parser& parse_u64(u64& val)
    {
        val = *(u64*)ptr;
        ptr += sizeof(u64);
        return *this;
    }

    Parser& parse_str(str& val)
    {
        val.assign((const char*)ptr);
        ptr += val.size() + 1;
        
        return *this;
    }

    Parser& parser_i32(i32& val)
    {
        val = *(i32*)ptr;
        ptr += sizeof(i32);
        return *this;
    }

    template<typename Col>
    Parser& parse_col(Col& col)
    {   
        u64 size;
        parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& c = col.emplace_back();
            c.parse(*this);
        }

        return *this;
    }
};

std::optional<Message> getFinalMessage(int socket);
void sendFinalMessage(int socket, const Message& msg);

using Callback = std::function<void(Message&)>;

class Connection
{
protected:
    u32 nextReqId = 1;
    std::unordered_map<u32,Callback> callbacks;

    u32 getNextReqId()
    {
        u32 next = nextReqId;
        nextReqId++;

        if(nextReqId == 0)
            nextReqId++;

        return next;
    }

public:
    virtual void sendMessage(Message& msg, std::optional<Callback> callback = std::nullopt) = 0;
    virtual void sendMessage(Message&& msg, std::optional<Callback> callback = std::nullopt) = 0;
    virtual std::optional<Message> getMessage() = 0;
};

class ServerConnection : public Connection
{
    int serverFileDescriptor;
    std::vector<int> sockets;
    sockaddr_in address;

public:
    void init(const IpInfo& ip)
    {
        log("Initializing ServerConnection address=%, port=%", ip.address, ip.port);

        if ((serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        { 
            perror("socket failed"); 
        } 
        
        address.sin_family = AF_INET; 
        address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0
        address.sin_port = htons(ip.port); 
        
        if (bind(serverFileDescriptor,(sockaddr *)&address, sizeof(address)) < 0) 
        {
            std::cout << "Failed to Bind" << std::endl;
        }
        if (listen(serverFileDescriptor, 3) < 0) 
        {
            std::cout << "Failed to Listen" << std::endl;
        }
    }

    void acceptNewConnections(bool wait = false)
    {
        int new_socket;
        int addrlen = sizeof(address);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;

        fd_set read_fd_set;
        FD_ZERO(&read_fd_set); 
        FD_SET(serverFileDescriptor, &read_fd_set);

        if(!wait)
        {
            int retval;
            retval = select(serverFileDescriptor+1, &read_fd_set, NULL, NULL, &timeout);
            if(retval <= 0)
            {
                return;
            }
        }

        if((new_socket = accept(serverFileDescriptor, (sockaddr *)&address, (socklen_t*)&addrlen)) >= 0)
        {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100;
            setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            setsockopt(new_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
            sockets.push_back(new_socket);
            log("ServerConnection: accepted new connection socket=%", new_socket);
        }
    }

    void sendMessage(Message& msg, std::optional<Callback> callback = std::nullopt) override
    {
        for(int socket : sockets)
        {
            if(msg.reqId == 0)
            {
                msg.reqId = getNextReqId();
            }
            if(callback.has_value())
            {
                callbacks[msg.reqId] = callback.value();
            }

            sendFinalMessage(socket, msg);
        }
    }

    void sendMessage(Message&& msg, std::optional<Callback> callback = std::nullopt) override
    {
        sendMessage(msg, callback);
    }

    void sendMessage(int socket, Message& msg, std::optional<Callback> callback = std::nullopt)
    {
        if(msg.reqId == 0)
        {
            msg.reqId = getNextReqId();
        }
        if(callback.has_value())
        {
            callbacks[msg.reqId] = callback.value();
        }

        sendFinalMessage(socket, msg);
    }

    void sendMessage(int socket, Message&& msg, std::optional<Callback> callback = std::nullopt)
    {
        sendMessage(socket, msg, callback);
    }

    std::optional<Message> getMessage()
    {
        for(int socket : sockets)
        {
            std::optional<Message> potentialMsg = getFinalMessage(socket);
            if(potentialMsg.has_value())
            {
                Message& msg = potentialMsg.value();

                auto it = callbacks.find(msg.reqId);
                if(it != callbacks.end())
                {
                    it->second(msg);
                    callbacks.erase(it);
                    return std::nullopt; // Callback will process Message
                }
                else
                {
                    return potentialMsg; // Module will process Message
                }
            }
        }

        return std::nullopt;
    }
};

class ClientConnection : public Connection
{
    int socketFileDescriptor;

public:
    void init(const IpInfo& ip)
    {
        log("Initializing ClientConnection address=%, port=%", ip.address, ip.port);
        sockaddr_in serv_addr;

        socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFileDescriptor < 0)
        { 
            log("Socket creation error");
            return;
        } 
    
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(ip.port); 
        
        if(inet_pton(AF_INET, ip.address.c_str(), &serv_addr.sin_addr) <= 0)  
        { 
            log("Invalid address/ Address not supported");
            return; 
        } 
    
        if (connect(socketFileDescriptor, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            log("Connection Failed");
            return; 
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        setsockopt(socketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        setsockopt(socketFileDescriptor, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

        log("Connection established");
    }

    int getSocket() const
    {
        return socketFileDescriptor;
    }

    void sendMessage(Message& msg, std::optional<Callback> callback = std::nullopt) override
    {
        if(msg.reqId == 0)
        {
            msg.reqId = getNextReqId();
        }
        if(callback.has_value())
        {
            callbacks[msg.reqId] = callback.value();
        }

        sendFinalMessage(socketFileDescriptor, msg);
    }

    void sendMessage(Message&& msg, std::optional<Callback> callback = std::nullopt) override
    {
        sendMessage(msg, callback);
    }

    std::optional<Message> getMessage() override
    {
        std::optional<Message> potentialMsg = getFinalMessage(socketFileDescriptor);
        if(potentialMsg.has_value())
        {
            Message& msg = potentialMsg.value();
            auto it = callbacks.find(msg.reqId);
            if(it != callbacks.end())
            {
                it->second(msg);
                callbacks.erase(it);
                return std::nullopt; // Callback will process Message
            }
            else
            {
                return potentialMsg; // Module will process Message
            }
        }
        else
        {
            return std::nullopt;
        }
    }
};