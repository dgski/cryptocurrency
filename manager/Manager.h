#include <set>

#include "../shared/Module.h"

constexpr u32 ASK_FOR_TRANS_FREQ = 60 * ONE_SECOND;

class Manager : public Module
{
    std::list<Block> chain;
    std::map<str, u64> wallets;

    Block currentBlock;
    std::map<str, i64> currentBlockWalletDeltas;

    u64 currentBaseHash;

    ServerConnection connFromMiners;
    ServerConnection connFromTransactioner;
    ServerConnection connFromNetworker;

    std::optional<RSAKeyPair> walletKeys;
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
    
    // Networker related
    void processPotentialWinningBlock(const Message& msg);
    void processPotentialWinningBlock_ChainReply(const Message& msg);
    void processPotentialWinningBlock_Finalize(const std::set<u64>& transactionHashes);
    void processNetworkerChainRequest(const Message& msg);

    static std::optional<std::set<u64>> getValidTransHashes(std::vector<Block>& chain);

    void pushBlock(Block& block);

    void tryAbsorbChain(i32 orginSocket, std::list<Block> potentialChain);
    void finalizeAbsorbChain(std::list<Block> chain);
};