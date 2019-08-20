#include "../shared/Module.h"

class Manager : public Module
{
    //std::vector<Transaction> postedTransactions;
    
    Block currentBlock;
    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    ServerConnection connFromNetworker;
public:
    Manager(const char* iniFileName);
    void processMessage(Message& msg);
    void askTransactionerForNewTransactions();
    void processTransactionRequestReply(Message& msg);
    void sendBaseHashToMiners();
};