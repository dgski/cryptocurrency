#pragma once

#include <atomic>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include "Types.h"
#include "../simple-time/SimpleTime.h"

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

bool validProof(u64 nonce, u64 hash);

template<typename T, typename H>
size_t hashVector(const std::vector<T>& data, H hashFunction)
{
    size_t seed = data.size();
    for(auto d : data)
    {
        seed ^= 3203302344 ^ hashFunction(d);
    }

    return seed;
}

struct IpInfo
{
    str address;
    u32 port;
};

IpInfo strToIp(str s);

std::vector<str> splitStr(const str& input);

u64 getCurrentUnixTime();

constexpr u64 ONE_SECOND = 1000000;

struct pairHash
{
    template<typename A, typename B>
    size_t operator()(const std::pair<A,B> p) const
    {
        return 3203302344 ^ std::hash<A>{}(p.first) ^ std::hash<B>{}(p.second) << 22;
    }
};

