#include <iostream>
#include <thread>

#include "../shared/Communication.h"

class Manager
{
    ServerConnection miners;
public:
    Manager(const char* ip, const char* port)
    {
        miners.start(ip, atoi(port));
    }

    void run()
    {
        while(true)
        {
            //std::cout << "Cycle" << std::endl;

            // Check for new Connections
            auto v = miners.accept_new_connections();
            for(int socket : v)
            {
                Message msg;
                msg.id = 1;
                msg.compose_u64(12393939334343);
                sendMessage(socket, msg);
            }

            // Check for new Messages
            for(int s : miners.sockets)
            {
                std::optional<Message> msg = getMessage(s);
                if(msg.has_value())
                {
                    Parser parser(msg.value());
                    u64 proofOfWork;
                    parser.parse_u64(proofOfWork);
                    std::cout << "Received Proof of Work:" << proofOfWork << std::endl;
                    return;
                }
            }            

            //std::cout << "FakeManager running. Send new hash?" << std::endl;
            //char c;
            //std::cin >> c;
            /*
            if(c == 'y')
            {
                std::cout << "Sending New Hash" << std::endl;
                Message msg;
                msg.id = 1;
                msg.compose_u64(100);

                for(int s : miners.sockets)
                    sendMessage(s, msg);
            }
            */

            std::this_thread::sleep_for (std::chrono::milliseconds(1));
        }
    }
};