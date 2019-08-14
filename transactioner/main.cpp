#include "Transactioner.h"

int main(int argc, const char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Improper Usage" << std::endl;
        return -1;
    }

    Transactioner transactioner{argv[1]};
    transactioner.run2();
    return 0;
}