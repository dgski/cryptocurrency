#include "../shared/Module.h"

class Manager : public Module
{
    std::vector<Transaction> postedTransactions;
    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    
public:
    Manager(const char* iniFileName);
    void processMinerMessage(Message& msg);
    void askTransactionerForNewTransactions();
    void processTransactionRequestReply(Message& msg);
    void processMessage(Message& msg);
};