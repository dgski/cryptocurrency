#include "../shared/Module.h"

class Miner : Module
{
    ClientConnection connToManager;

    bool currentlyMining = false;
    AtomicChannel<u64> proof;
    AtomicChannel<u64> baseHash;
public:
    Miner(const char* iniFileName);
    void run();
    bool validProof(u64 nonce, u64 hash) const;
    void startMining();
    void stopMining();
    void mine();
    void processManagerMessage(Message& msg);
};