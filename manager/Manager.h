#include <iostream>

#include "../shared/Message.h"

class Manager
{
    int new_socket;
public:
    Manager()
    {
        int server_fd, valread; 
        struct sockaddr_in address; 
        int opt = 1; 
        int addrlen = sizeof(address); 
        
        // Creating socket file descriptor 
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        { 
            perror("socket failed"); 
            exit(EXIT_FAILURE); 
        } 
        
        address.sin_family = AF_INET; 
        address.sin_addr.s_addr = INADDR_ANY; 
        address.sin_port = htons( 6001 ); 
        
        if (bind(server_fd, (struct sockaddr *)&address,  
                                    sizeof(address))<0) 
        { 
            perror("bind failed"); 
            exit(EXIT_FAILURE); 
        } 
        if (listen(server_fd, 3) < 0) 
        { 
            perror("listen"); 
            exit(EXIT_FAILURE); 
        } 
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        }
    }

    void run()
    {
        while(true)
        {
            int x = GetMessage();

            std::cout << "FakeManager running. Send new hash?" << std::endl;
            char c;
            std::cin >> c;
            if(c == 'y')
            {
                Message msg;
                SendMessage(msg);
            }
        }
    }

    int GetMessage()
    {
        return 0;
    }

    void SendMessage(Message& msg)
    {
        std::vector<byte> data;
        data.resize(20);

        u32 id = 1;
        u64 size = 8; 
        u64 newBaseHash = 100;
        memcpy(data.data(), &id, 4);
        memcpy(data.data() + 4, &size, 8);
        memcpy(data.data() + 12, &newBaseHash, 8);

        for(byte b : msg.data)
            std::cout << (char)b << " ";
        std::cout << std::endl;

        send(new_socket , data.data() , data.size(), 0); 
        printf("message sent\n"); 
    }
};