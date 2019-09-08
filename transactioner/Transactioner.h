#include "../shared/Module.h"

class Transactioner : public Module
{
    ClientConnection connToManager;
    ServerConnection connFromClients;

    std::list<Transaction> waitingTransactions;

public:
    Transactioner(const char* iniFileName);
    void processMessage(const Message& msg);
    void processRequestForTransactions(const Message& msg);
    void processAddNewTransaction(const Message& msg);
    void processAddNewTransaction_Step2(const Message& msg, const Transaction& transaction);
};