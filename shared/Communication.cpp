#include "Communication.h"

std::optional<Message> getMessage(int socket)
{
    Message msg;
    
    int bytesRead = read(socket, &msg.id, 4);
    if(bytesRead == 0)
    {
        return std::nullopt;
    }

    bytesRead = read(socket, &msg.size, 8);
    if(bytesRead == 0)
    {
        return std::nullopt;
    }
    
    msg.data.resize(msg.size);
    bytesRead = read(socket, msg.data.data(), msg.size);
    if(bytesRead == 0)
    {
        return std::nullopt;
    }
    
    return msg;
}

void sendMessage(int socket, Message& msg)
{
    std::vector<byte> buffer;
    buffer.resize(msg.getFullSize());
    
    memcpy(buffer.data(), &msg.id, 4);
    memcpy(buffer.data() + 4, &msg.size, 8);
    memcpy(buffer.data() + 12, msg.data.data(), msg.size);

    int bytes = send(socket, buffer.data(), buffer.size(), 0);
}