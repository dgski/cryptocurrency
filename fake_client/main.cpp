#include "../shared/Module.h"

int main()
{
    ClientConnection connToTransactioner;
    connToTransactioner.init("0.0.0.0", 8000);

    while(true)
    {
        std::cout << "Send Fake Transaction >>" << std::endl;
        char x;
        std::cin >> x;
        if(x == 'y')
        {
            Transaction t;
            t.id = 19292;

            MSG_CLIENT_TRANSACTIONER_NEWTRANS contents;
            Message msg;
            contents.compose(msg);

            sendMessage(connToTransactioner.getSocket(), msg);
        }
    }
}