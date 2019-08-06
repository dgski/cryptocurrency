#include "../shared/Module.h"

class Manager : Module
{
    std::vector<Transaction> postedTransactions;
    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    
public:
    Manager(const char* iniFileName);
    void run();
    void processMinerMessage(Message& msg);
    void processTransactionerMessage(Message& msg);
    template<typename T>
    static size_t hashVector(std::vector<T> data);
};