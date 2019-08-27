#include "../shared/Module.h"

class Manager : public Module
{
    // temporary blockchain
    std::vector<Block> chain;

    Block currentBlock;
    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    ServerConnection connFromNetworker;
public:
    Manager(const char* iniFileName);
    void processMessage(const Message& msg);
    void askTransactionerForNewTransactions();
    void processTransactionRequestReply(Message& msg);
    void sendBaseHashToMiners();
    void processIncomingProofOfWork(const Message& msg);
};