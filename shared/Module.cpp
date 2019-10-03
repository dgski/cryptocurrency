#include "Module.h"

void Module::run()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::vector<EnquedMessage> incomingQueue;
        try
        {
            for(ServerConnection* s : serverConnections)
            {
                s->getIncomingQueue(incomingQueue);
                for(auto& msg : incomingQueue)
                {
                    if(msg.callback.has_value())
                    {
                        msg.callback.value()(msg.message);
                    }
                    else
                    {
                        processMessage(msg.message);
                    }
                }
                incomingQueue.clear();
            }

            for(ClientConnection* c : clientConnections)
            {
                c->getIncomingQueue(incomingQueue);
                for(auto& msg : incomingQueue)
                {
                    if(msg.callback.has_value())
                    {
                        msg.callback.value()(msg.message);
                    }
                    else
                    {
                        processMessage(msg.message);
                    }
                }
                incomingQueue.clear();
            }

            u64 now = getCurrentUnixTime();
            for(auto it = scheduledTasks.begin(); it != scheduledTasks.end(); ++it)
            {
                if(it->when <= now)
                {
                    std::invoke(it->task);
                    scheduledTasks.erase(it);
                }
            }
        }
        catch(const std::exception& e)
        {
            logger.logError({
                {"event", "exception"},
                {"description", e.what()}
            });
        }
    }
}