#include <list>

#include "../shared/Module.h"

class Transactioner : public Module
{
    ClientConnection connToManager;
    ServerConnection connFromClients;

    std::vector<Transaction> waitingTransactions;

public:
    Transactioner(const char* iniFileName);
    void processMessage(const Message& msg);
    void processRequestForTransactions(const Message& msg);
    void processAddNewTransaction(const Message& msg);
};