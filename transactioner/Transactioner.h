#include <list>

#include "../shared/Module.h"

class Transactioner : Module
{
    ClientConnection connToManager;
    ServerConnection connFromClients;

    std::vector<Transaction> waitingTransactions;

public:
    Transactioner(const char* iniFileName);
    void run();
    void processManagerMessage(Message& msg);
    void processClientMessage(Message& msg);
};