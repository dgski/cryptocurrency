#include "Manager.h"

int main(int argc, const char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Improper Usage" << std::endl;
        return -1;
    }

    Manager manager{argv[1], argv[2]};
    manager.run();
    return 0;
}