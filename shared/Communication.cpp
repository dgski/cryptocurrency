#include "Communication.h"

std::optional<Message> getFinalMessage(int socket)
{
    Message msg;
    msg.socket = socket;
    
    int bytesRead = read(socket, &msg.id, 4);
    if(bytesRead <= 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.reqId, 4);
    if(bytesRead <= 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.size, 8);
    if(bytesRead <= 0)
    {
        return std::nullopt;
    }
    if(msg.size == 0)
    {
        return msg;
    }

    msg.data.resize(msg.size);
    bytesRead = read(socket, msg.data.data(), msg.size);
    if(bytesRead == 0)
    {
        return msg;
    }
    if(bytesRead < 0)
    {
        return std::nullopt;
    }
    
    return msg;
}

ConnectionStatus sendFinalMessage(int socket, const Message& msg)
{
    std::vector<byte> buffer(msg.getFullSize());
    
    memcpy(buffer.data(), &msg.id, 4);
    memcpy(buffer.data() + 4, &msg.reqId, 4);
    memcpy(buffer.data() + 8, &msg.size, 8);
    memcpy(buffer.data() + 16, msg.data.data(), msg.size);

    char dummy;
    const int open = recv(socket, &dummy, 1, MSG_PEEK);
    if(open == 0)
    {
        return ConnectionStatus::Disconnected;
    }

    const int bytes = send(socket, buffer.data(), buffer.size(), 0);
    return ConnectionStatus::Connected;
}