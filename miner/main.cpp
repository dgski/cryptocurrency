#include "Miner.h"

int main(int argc, const char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Impropter Usage" << std::endl;
        return -1;
    }

    Miner miner{argv[1], argv[2]};
    miner.run();
    return 0;
}