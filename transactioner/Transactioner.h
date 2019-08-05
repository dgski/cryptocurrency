#include "../shared/Module.h"

class Transactioner : Module
{
    ClientConnection connToManager;
    ServerConnection connFromClients;
public:
    Transactioner(const char* iniFileName)
    {
        std::cout << "Transactioner Module Starting" << std::endl;

        std::map<str,str> params = getInitParameters(iniFileName);

        connToManager.init(
            params["connToManagerIP"].c_str(),
            atoi(params["connToManagerPORT"].c_str())
        );

        connFromClients.init(
            params["connFromClientsIP"].c_str(),
            atoi(params["connFromClientsPORT"].c_str())
        );
    }
    void run()
    {
        while(true)
        {
            std::cout << "Cycle" << std::endl;
            std::optional<Message> msg = getMessage(connToManager.getSocket());
            if(msg.has_value())
            {
                Parser parser(msg.value());
                if(msg.value().id == 3)
                {
                    std::cout << "Manager Asking For New Transactions" << std::endl;
                    Message msg;
                    msg.id = 4;
                    sendMessage(connToManager.getSocket(), msg);                    
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};