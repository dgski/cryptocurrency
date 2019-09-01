#include "Module.h"

void Module::run()
{
    while(true)
    {
        for(ServerConnection* s : serverConnnections)
        {
            s->acceptNewConnections();
            std::optional<Message> msg = s->getMessage();
            if(msg.has_value())
            {
                processMessage(msg.value());
            }
        }

        for(ClientConnection* c : clientConnections)
        {
            std::optional<Message> msg = c->getMessage();
            if(msg.has_value())
            {
                processMessage(msg.value());
            }
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
}