#include "../shared/Module.h"

class Manager : Module
{
    AtomicChannel<u64> currentBaseHash;
    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
public:
    Manager(const char* iniFileName)
    {
        std::cout << "Manager Module Starting" << std::endl;

        std::map<str,str> params = getInitParameters(iniFileName);

        connFromMiners.init(
            params["connFromMinersIP"].c_str(),
            atoi(params["connFromMinersPORT"].c_str())
        );

        connFromTransactioner.init(
            params["connFromTransactionerIP"].c_str(),
            atoi(params["connFromTransactionerPORT"].c_str()) 
        );
        
        currentBaseHash.set(12393939334343); // FAKE
    }

    void run()
    {
        connFromTransactioner.acceptNewConnections(true);

        while(true)
        {
            std::cout << "Cycle" << std::endl;

            // Connect to New Miners
            std::vector<int> newConnections = connFromMiners.acceptNewConnections();
            u64 newHashToSend = currentBaseHash.get();
            for(int socket : newConnections)
            {
                Message msg;
                msg.id = 1;
                msg.compose_u64(newHashToSend);
                sendMessage(socket, msg);
            }
            
            // Check if any miner has send the proof of work
            for(int s : connFromMiners.sockets)
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

            // Ask Transactioner for new transactions
            for(int socket : connFromTransactioner.sockets)
            {
                Message msg;
                msg.id = 3;
                sendMessage(socket, msg);
            }

            // Transactioner sent new transactions
            for(int socket : connFromTransactioner.sockets)
            {
                std::optional<Message> msg = getMessage(socket);
                if(msg.has_value())
                {
                    std::cout << "Received new Transactions from Transactioner" << std::endl;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};