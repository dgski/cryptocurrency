#include "../shared/Module.h"

class Networker : public Module
{
    ClientConnection connToManager;
public:
    Networker(const char* iniFileName);
    void processMessage(Message& msg);
};