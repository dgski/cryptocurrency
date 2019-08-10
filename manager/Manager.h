#include "../shared/Module.h"

class Manager : public Module
{
    std::vector<Transaction> postedTransactions;
    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    
public:
    Manager(const char* iniFileName);
    void run();
    void processMinerMessage(Message& msg);
    void sendNewBaseHashToMiners(const std::vector<int>& sockets) const;
    void askTransactionerForNewTransactions();
    void processTransactionRequestReply(Message& msg);
};