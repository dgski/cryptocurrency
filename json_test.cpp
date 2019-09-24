#include "shared/Logger.h"
#include "shared/Types.h"

#include <fstream>

int main()
{
    str res = toJSONStr({
        {"time", 1231232},
        {"event", "incoming message"},
        {"reqId", 12332}
    });

    Logger logger;

    std::ofstream file{"test.txt", std::ios_base::openmode::_S_app};
    if(!file.is_open())
    {
        std::cout << "Could not open file!" << std::endl;
        return -1;
    }
    logger.streams.push_back(&std::cout);
    logger.streams.push_back(&file);

    logger.push(res);
}