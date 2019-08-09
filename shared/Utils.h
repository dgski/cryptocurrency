#pragma once

#include <atomic>
#include <map>
#include <vector>

#include "Types.h"

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

std::map<str, str> getInitParameters(const char* fileName);

template<typename T>
size_t hashVector(std::vector<T> data);

bool validProof(u64 nonce, u64 hash);

template<typename T>
size_t hashVector(std::vector<T> data)
{
    size_t seed = data.size();
    for(auto d : data)
    {
        seed ^= 3203302344 ^ std::hash<T>{}(d);
    }

    return seed;
}