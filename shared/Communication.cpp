#include "Communication.h"

std::optional<Message> getFinalMessage(i32 socket)
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

ConnectionStatus sendFinalMessage(i32 socket, const Message& msg)
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

void ServerConnection::init(const IpInfo& ip)
{
    logger.logInfo({
        {"event", "Initializing ServerConnection"},
        {"ip.address", ip.address},
        {"ip.port", ip.port}
    });

    outgoingMutex = std::unique_ptr<std::mutex>(new std::mutex());
    incomingMutex = std::unique_ptr<std::mutex>(new std::mutex());
    socketsMutex = std::unique_ptr<std::mutex>(new std::mutex());

    if ((serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
    } 
    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0
    address.sin_port = htons(ip.port); 
    
    if (bind(serverFileDescriptor,(sockaddr *)&address, sizeof(address)) < 0) 
    {
        logger.logError("Failed to Bind");
        return;
    }
    if (listen(serverFileDescriptor, 3) < 0) 
    {
        logger.logError("Failed to Listen");
        return;
    }

    logger.logInfo("ServerConnection established");

    std::thread(&ServerConnection::runOutgoing, this).detach();
    std::thread(&ServerConnection::runIncoming, this).detach();
}

void ServerConnection::acceptNewConnections(const bool wait)
{
    int new_socket;
    int addrlen = sizeof(address);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

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

    new_socket = accept(serverFileDescriptor, (sockaddr *)&address, (socklen_t*)&addrlen);
    if(new_socket >= 0)
    {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        setsockopt(new_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
        
        {
            std::lock_guard<std::mutex> lock(*socketsMutex);
            sockets.push_back(new_socket);
        }

        logger.logInfo({
            {"event", "ServerConnection: accepted new connection socket"},
            {"new_socket", new_socket}
        });
    }
}

void ServerConnection::runOutgoing()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if(!savedMessages.empty())
        {
            for(auto it = std::begin(savedMessages); it != std::end(savedMessages); ++it)
            {
                auto status = sendToSingle(*it);
                if(status != ConnectionStatus::MessageNotSent)
                {
                    savedMessages.erase(it);
                }
            }
        }

        std::vector<EnquedMessage> currentQueue;
        {
            std::lock_guard<std::mutex> lock(*outgoingMutex);
            if(outgoingQueue.empty())
            {
                continue;
            }
            currentQueue.swap(outgoingQueue);
        }
        
        for(auto& msg : currentQueue)
        {
            msg.message.logMsg("outgoing");
            if(msg.socket.has_value())
            {
                if(sendToSingle(msg) == ConnectionStatus::MessageNotSent)
                {
                    savedMessages.push_back({msg.socket.value(), std::move(msg.message), std::nullopt});
                }
            }
            else
            {
                sendToAll(msg);
            }
        }
    }
}

void ServerConnection::runIncoming()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        acceptNewConnections();
        std::lock_guard<std::mutex> lock(*socketsMutex);
        for(int socket : sockets)
        {
            std::optional<Message> potentialMsg = getFinalMessage(socket);
            if(potentialMsg.has_value())
            {
                Message& msg = potentialMsg.value();
                msg.logMsg("incoming");

                std::lock_guard<std::mutex> lock(*incomingMutex);
                incomingQueue.push_back({
                    socket,
                    std::move(msg),
                    std::move(getCallback(socket, msg.reqId))});
            }
        }
    }
}

void ServerConnection::sendToAll(EnquedMessage& msg)
{
    const bool generateRequestId = msg.message.reqId == 0;

    for(auto it = begin(sockets); it != end(sockets); ++it)
    {
        if(generateRequestId)
        {
            msg.message.reqId = getNextReqId();
        }
        else
        {
            msg.message.isReply = true;
        }

        if(msg.callback.has_value())
        {
            callbacks[std::pair{*it, msg.message.reqId}] = msg.callback.value();
        }

        msg.message.logMsg("outgoing");

        auto status = sendFinalMessage(*it, msg.message);
        if(status == ConnectionStatus::Disconnected)
        {
            logger.logWarning({
                {"event", "Connection closed; could not send message. Deleting."},
                {"socket", *it}
            });
            
            std::lock_guard<std::mutex> lock(*socketsMutex);
            sockets.erase(it);
        }
        else if(status == ConnectionStatus::MessageNotSent)
        {
            savedMessages.push_back({*it, msg.message, std::nullopt});
        }
    }
}

ConnectionStatus ServerConnection::sendToSingle(EnquedMessage& msg)
{
    if(msg.message.reqId == 0)
    {
        msg.message.reqId = getNextReqId();
    }
    else
    {
        msg.message.isReply = true;
    }

    if(msg.callback.has_value())
    {
        callbacks[std::pair{msg.socket.value(), msg.message.reqId}] = msg.callback.value();
    }

    return sendFinalMessage(msg.socket.value(), msg.message);
}

void ClientConnection::init(const IpInfo& ip)
{
    logger.logInfo({
        {"event", "Initializing ClientConnection"},
        {"ip.address", ip.address},
        {"ip.port", ip.port}
    });

    outgoingMutex = std::unique_ptr<std::mutex>(new std::mutex());
    incomingMutex = std::unique_ptr<std::mutex>(new std::mutex());

    sockaddr_in serv_addr;

    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0)
    { 
        logger.logError("Socket creation error");
        return;
    } 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(ip.port); 
    
    if(inet_pton(AF_INET, ip.address.c_str(), &serv_addr.sin_addr) <= 0)  
    { 
        logger.logError("Invalid address/ Address not supported");
        return; 
    } 

    if (connect(socketFileDescriptor, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        logger.logError("Connection Failed");
        return; 
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(socketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(socketFileDescriptor, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    logger.logInfo("ClientConnection established");

    std::thread(&ClientConnection::runOutgoing, this).detach();
    std::thread(&ClientConnection::runIncoming, this).detach();
}

void ClientConnection::runOutgoing()
{
    std::list<Message> savedMessages;
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        for(auto it = std::begin(savedMessages); it != std::end(savedMessages); ++it)
        {
            auto status = sendFinalMessage(socketFileDescriptor, *it);
            if(status != ConnectionStatus::MessageNotSent)
            {
                savedMessages.erase(it);
            }
        }

        std::vector<EnquedMessage> currentQueue;
        {
            std::lock_guard<std::mutex> lock(*outgoingMutex);
            if(outgoingQueue.empty())
            {
                continue;
            }
            currentQueue.swap(outgoingQueue);
        }

        for(auto& msg : currentQueue)
        {
            msg.message.logMsg("outgoing");
            if(msg.message.reqId == 0)
            {
                msg.message.reqId = getNextReqId();
            }
            else
            {
                msg.message.isReply = true;
            }

            if(msg.callback.has_value())
            {
                callbacks[std::pair{socketFileDescriptor, msg.message.reqId}] = msg.callback.value();
            }

            auto status = sendFinalMessage(socketFileDescriptor, msg.message);
            if(status == ConnectionStatus::MessageNotSent)
            {
                savedMessages.push_back(std::move(msg.message));
            }
        }
        
    }
}

void ClientConnection::runIncoming()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::optional<Message> potentialMsg = getFinalMessage(socketFileDescriptor);
        if(potentialMsg.has_value())
        {
            Message& msg = potentialMsg.value();
            msg.logMsg("incoming");

            std::lock_guard<std::mutex> lock(*incomingMutex);
            incomingQueue.push_back({
                socketFileDescriptor,
                std::move(msg),
                std::move(getCallback(socketFileDescriptor, msg.reqId))});
        }
    }
}