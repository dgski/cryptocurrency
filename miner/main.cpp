#include "Miner.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        log("Improper Usage");
        return -1;
    }

    Miner miner{argv[1]};
    miner.run();
    return 0;
}