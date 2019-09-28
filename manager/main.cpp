#include "Manager.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        logger.logError("Improper Usage");
        return -1;
    }

    Manager manager{ argv[1] };
    manager.run();
    return 0;
}