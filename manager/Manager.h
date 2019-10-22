#include <set>

#include "../shared/Module.h"

constexpr u32 ASK_FOR_TRANS_FREQ = 60 * ONE_SECOND;

class Manager : public Module
{
    std::list<Block> chain;
    std::map<u64, std::list<Block>::iterator> hashToBlock;
    std::map<u64, std::list<Block>::iterator> idToBlock;
    std::map<str, i64> wallets;

    Block currentBlock;
    std::map<str, i64> currentBlockWalletDeltas;

    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    ServerConnection connFromNetworker;

    std::optional<RSAKeyPair> walletKeys;
    u32 chainValidationCapacity = 10;
public:
    Manager(const char* iniFileName);
    void processMessage(const Message& msg);
    
    void mintCurrency();

    // Transactioner related
    void askTransactionerForNewTransactions();
    void processTransactionRequestReply(const Message& msg);
    void addTransactionToCurrentBlock(Transaction& t);
    void processTransactionWalletInquiry(const Message& msg);
    
    // Miner related
    void sendBaseHashToMiners();
    void processIncomingProofOfWork(const Message& msg);
    void processMinerHashRequest(const Message& msg);
    void pushBlock(Block& block);
    
    // Networker related
    void sendVoidBlockRequest(const u32 reqId);
    void processPotentialWinningBlock(const Message& msg);
    void tryAbsorbChain(u32 reqId, std::list<Block> potentialChain);
    void finalizeAbsorbChain(std::list<Block> chain);
    void removeTransactionFromCurrentBlock(const Transaction& t);
    void processNetworkerBlockRequest(const Message& msg);

    // Utils
    static void addTransactionToWallets(std::map<str, i64>& wallets, const Transaction& t);
    static void removeTransactionFromWallets(std::map<str, i64>& wallets, const Transaction& t);
};