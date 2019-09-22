#include <chrono>

#include "../shared/Module.h"

int main()
{
    auto keys = RSAKeyPair::create("private.pem", "public.pem");
    if(!keys.has_value())
    {
        log("Could not create keypair!");
        return -1;
    }

    ClientConnection connToTransactioner;
    connToTransactioner.init(IpInfo{"0.0.0.0",8000});

    while(true)
    {
        std::cout << "Send Fake Transaction >>" << std::endl;
        int x;
        std::cin >> x;
        if(0 < x)
        {
            Transaction t;
            t.time = getCurrentUnixTime();
            t.sender = keys.value().publicKey;
            t.recipiant = keys.value().publicKey;
            t.amount = 10;
            t.sign(keys.value());

            MSG_CLIENT_TRANSACTIONER_NEWTRANS outgoing;
            outgoing.transaction = t;
            
            Message msg;
            outgoing.compose(msg);

            connToTransactioner.sendMessage(msg);
        }
    }
}