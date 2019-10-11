#include <chrono>

#include "../shared/Module.h"

int main()
{
    logger.addOutputStream(&std::cout);
    logger.run();
    logger.logInfo("Starting Fake Client!");

    auto keys = RSAKeyPair::create("test_keys/private.pem", "test_keys/public.pem");
    if(!keys.has_value())
    {
        logger.logError("Could not create keypair!");
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

            u64 count;
            while(count < x)
            {
                std::cout << count++ << std::endl;
                connToTransactioner.sendMessage(msg);
            }
        }
    }
}