#pragma once

#include <atomic>
#include <map>
#include <vector>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

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

inline void log(const char* fmt)
{
    std::cout << fmt << std::endl;
}

template<typename Arg, typename... Args>
inline void log(const char* fmt, Arg arg, Args... args)
{
    while(*fmt)
    {
        if(*fmt == '%')
        {
            std::cout << arg;
            log(fmt + 1, args...);
            return;
        }

        std::cout << *fmt;
        fmt += 1;
    }
}