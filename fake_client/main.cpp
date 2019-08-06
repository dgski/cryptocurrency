#include "../shared/Module.h"

int main()
{
    ClientConnection connToTransactioner;
    connToTransactioner.init("0.0.0.0", 8000);

    while(true)
    {
        std::cout << "Send Fake Transaction >>" << std::endl;
        int x;
        std::cin >> x;
        if(0 < x)
        {
            Transaction t;
            t.id = x;

            MSG_CLIENT_TRANSACTIONER_NEWTRANS contents;
            Message msg;
            contents.compose(msg);

            sendMessage(connToTransactioner.getSocket(), msg);
        }
    }
}