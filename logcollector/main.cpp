#include "LogCollector.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        return -1;
    }

    LogCollector logcollector{argv[1]};
    logcollector.run();
    return 0;
}