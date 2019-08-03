#include <iostream>
#include <atomic>
#include <thread>
#include <sstream>

using i32 = std::int32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

template<typename T>
struct AtomicChannel
{
    std::atomic<bool> waiting{false};
    std::atomic<T> value;

    bool ready()
    {
        return waiting.load();
    }

    T get()
    {
        if(ready() == false)
            throw std::runtime_error("Cannot get(): Atomic Channel Value is not Ready.");
        waiting.store(false);
        return value.load();
    }

    void set(T _value)
    {
        if(ready() == true)
            throw std::runtime_error("Cannot set(): Atomic Channel Value is Ready.");
        waiting.store(true);
        value.store(_value);
    }
};

class Miner
{
    bool currentlyMining = false;
    AtomicChannel<u64> proof;
    AtomicChannel<u64> baseHash;

public:
    Miner() {}
    void run();
    i32 getMessage();
    bool validProof(u64 nonce, u64 hash) const;
    void startMining();
    void stopMining();
    void mine();
};