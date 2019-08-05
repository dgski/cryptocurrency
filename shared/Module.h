#pragma once

#include <iostream>
#include <thread>
#include <optional>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"

class Module
{
public:
    virtual void run() = 0;
};