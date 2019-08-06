#pragma once

#include <iostream>
#include <thread>
#include <optional>

#include "../shared/Types.h"
#include "../shared/Utils.h"
#include "../shared/Communication.h"
#include "../shared/Transaction.h"
#include "../shared/Messages.h"

class Module
{
public:
    virtual void run() = 0;
};