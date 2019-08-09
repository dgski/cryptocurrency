#pragma once

#include <iostream>
#include <thread>
#include <optional>
#include <unordered_map>
#include <functional>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"
#include "../shared/Transaction.h"
#include "../shared/Messages.h"

class Module
{
    u32 nextReqId = 1;
protected:
    std::unordered_map<u32,std::function<void(Message& msg)>> callBacks;
    u32 getNextReqId()
    {
        return nextReqId++;
    }
public:
    virtual void run() = 0;
};