#include "Manager.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Improper Usage" << std::endl;
        return -1;
    }

    Manager manager{ argv[1] };
    //manager.run();
    manager.run2();
    return 0;
}