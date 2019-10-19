#include "Communication.h"

std::optional<Message> getFinalMessage(int socket)
{
    Message msg;
    msg.socket = socket;
    
    int bytesRead = read(socket, &msg.id, sizeof(u32));
    if(bytesRead <= 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.isReply, sizeof(bool));
    if(bytesRead <= 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.reqId, sizeof(u32));
    if(bytesRead <= 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.size, sizeof(u64));
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
    memcpy(buffer.data() + 4, &msg.isReply, 1);
    memcpy(buffer.data() + 5, &msg.reqId, 4);
    memcpy(buffer.data() + 9, &msg.size, 8);
    memcpy(buffer.data() + 17, msg.data.data(), msg.size);

    char dummy;
    const int open = recv(socket, &dummy, 1, MSG_PEEK);
    if(open == 0)
    {
        return ConnectionStatus::Disconnected;
    }

    const int bytes = send(socket, buffer.data(), buffer.size(), 0);
    if(bytes == -1)
    {
        return ConnectionStatus::MessageNotSent;
    }

    return ConnectionStatus::Connected;
}