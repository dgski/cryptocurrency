#include "Networker.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        logger.logError("Improper Usage");
        return -1;
    }

    Networker networker{argv[1]};
    networker.run();
    return 0;
}