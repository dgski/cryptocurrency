#include "../shared/Module.h"

class Miner : public Module
{
    ClientConnection connToManager;

    bool currentlyMining = false;
    AtomicChannel<u64> proof;
    AtomicChannel<u64> baseHash;
public:
    Miner(const char* iniFileName);
    void run();
    void startMining();
    void stopMining();
    void mine();
    void processManagerMessage(Message& msg);
};