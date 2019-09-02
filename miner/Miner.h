#include "../shared/Module.h"

class Miner : public Module
{
    ClientConnection connToManager;

    bool currentlyMining = false;
    AtomicChannel<u64> proof;
    AtomicChannel<u64> baseHash;
    u64 incrementSize;

public:
    Miner(const char* iniFileName);
    void processMessage(const Message& msg);
    void startMining();
    void stopMining();
    void mine();
    void checkProof();
    void processManagerNewBaseHash(const Message& msg);
    void requestNewBaseHash();
};