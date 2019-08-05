#pragma once

#include <atomic>
#include <map>

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