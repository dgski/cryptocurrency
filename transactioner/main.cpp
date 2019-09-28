#include "Transactioner.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        logger.logError("Improper Usage");
        return -1;
    }

    Transactioner transactioner{argv[1]};
    transactioner.run();
    return 0;
}