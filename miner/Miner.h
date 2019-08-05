#include <iostream>
#include <atomic>
#include <thread>
#include <sstream>
#include <optional>

#include "../shared/Communication.h"

template<typename T>
class AtomicChannel
{
    std::atomic<bool> waiting = false;
    std::atomic<T> value;
public:
    bool ready()
    {
        return waiting.load();
    }

    T get()
    {
        waiting.store(false);
        return value.load();
    }

    void set(T _value)
    {
        value.store(_value);
        waiting.store(true);
    }
};

class Miner
{
    i32 incomingFromManager;

    bool currentlyMining = false;
    AtomicChannel<u64> proof;
    AtomicChannel<u64> baseHash;
public:
    Miner(const char* ip, const char* port);
    void run();
    bool validProof(u64 nonce, u64 hash) const;
    void startMining();
    void stopMining();
    void mine();
};