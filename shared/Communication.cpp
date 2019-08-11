#include "Communication.h"

std::optional<Message> getFinalMessage(int socket)
{
    Message msg;
    
    int bytesRead = read(socket, &msg.id, 4);
    if(bytesRead < 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.reqId, 4);
    if(bytesRead < 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.size, 8);
    if(bytesRead < 0)
    {
        return std::nullopt;
    }

    msg.data.resize(msg.size);
    bytesRead = read(socket, msg.data.data(), msg.size);
    if(bytesRead < 0)
    {
        return std::nullopt;
    }

    msg.socket = socket;
    
    return msg;
}

void sendFinalMessage(int socket, Message& msg)
{
    std::vector<byte> buffer;
    buffer.resize(msg.getFullSize());
    
    memcpy(buffer.data(), &msg.id, 4);
    memcpy(buffer.data() + 4, &msg.reqId, 4);
    memcpy(buffer.data() + 8, &msg.size, 8);
    memcpy(buffer.data() + 16, msg.data.data(), msg.size);

    int bytes = send(socket, buffer.data(), buffer.size(), 0);
}