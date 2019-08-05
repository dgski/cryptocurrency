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

using i32 = std::int32_t;
using i64 = std::int64_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using byte = std::byte;

struct Message
{
    u32 id = 0;
    u64 size = 0;
    std::vector<byte> data;

    Message& compose_u64(u64 val)
    {
        size += sizeof(u64);
        data.resize(data.size() + sizeof(u64));
        memcpy(data.data(), &val, sizeof(u64));

        return *this;
    }

    size_t getFullSize() const
    {
        return sizeof(u32) + sizeof(u64) + data.size();
    }
};

struct Parser
{
    byte* ptr;
    Parser(Message& msg)
    {
        ptr = msg.data.data();
    }

    Parser& parse_u64(u64& val)
    {
        val = *(u64*)ptr;
        ptr += sizeof(u64);
        return *this;
    }
};

std::optional<Message> getMessage(int socket);
void sendMessage(int socket, Message& msg);

struct ServerConnection
{
    int serverFileDescriptor;
    std::vector<int> sockets;
    sockaddr_in address;

    void start(const char* ip, int port)
    {
        if ((serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        { 
            perror("socket failed"); 
        } 
        
        address.sin_family = AF_INET; 
        address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0
        address.sin_port = htons(port); 
        
        if (bind(serverFileDescriptor,(sockaddr *)&address, sizeof(address)) < 0) 
        {
            std::cout << "Failed to Bind" << std::endl;
        }
        if (listen(serverFileDescriptor, 3) < 0) 
        {
            std::cout << "Failed to Listen" << std::endl;
        }
    }

    std::vector<int> accept_new_connections()
    {
        std::vector<int> establishedThisRound;

        int new_socket;
        int addrlen = sizeof(address);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;

        fd_set read_fd_set;
        FD_ZERO(&read_fd_set); 
        FD_SET(serverFileDescriptor, &read_fd_set);

        int retval = select(serverFileDescriptor+1, &read_fd_set, NULL, NULL, &timeout);
        if(retval <= 0)
        {
            return {};
        }

        if((new_socket = accept(serverFileDescriptor, (sockaddr *)&address, (socklen_t*)&addrlen)) >= 0)
        {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 10;
            setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            sockets.push_back(new_socket);
            establishedThisRound.push_back(new_socket);
            std::cout << "+ New Incoming Connection" << std::endl;
        }

        return establishedThisRound;
    }
};