#include <chrono>

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
            t.time = std::chrono::system_clock::now().time_since_epoch().count();
            t.sender = "Bob";
            t.recipiant = "Michael";
            t.amount = 10;
            t.signature = "SIGNATURE";

            MSG_CLIENT_TRANSACTIONER_NEWTRANS contents;
            contents.transaction = t;
            
            Message msg;
            contents.compose(msg);

            connToTransactioner.sendMessage(msg);
        }
    }
}