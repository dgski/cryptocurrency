#pragma once

#include <tuple>
#include <initializer_list>
#include <queue>
#include <mutex>
#include <thread>

#include "Types.h"
#include "Utils.h"

class Loggable
{
public:
    str strValue;
    Loggable(const char* cstr)
    {
        strValue += '\"';
        strValue += cstr;
        strValue += '\"';
    }
    Loggable(int i)
    : strValue(std::to_string(i))
    {}
};

class MultiStream
{
    std::vector<std::ostream*> outputStreams;

public:
    MultiStream(std::initializer_list<std::ostream*> _outputStreams)
    : outputStreams(_outputStreams)
    {}

    template<typename T>
    MultiStream& operator<<(T obj)
    {
        for(std::ostream* output : outputStreams)
        {
            *output << obj;
        }

        return *this;
    }
};

class Logger
{
    MultiStream streams;

    std::mutex logQueueMutex;
    std::vector<std::vector<std::pair<const char*, Loggable>>> logQueue;

    std::thread outputThread;
    std::atomic<bool> waitingToDestruct = false;

public:
    Logger(std::initializer_list<std::ostream*> _streams)
    : streams(_streams),
    outputThread(&Logger::runOutput, this)
    {}

    void log(std::vector<std::pair<const char*, Loggable>> contents)
    {
        std::lock_guard<std::mutex> lock(logQueueMutex);
        logQueue.emplace_back(std::move(contents));
    }

    void runOutput()
    {
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            const bool onLastEntries = waitingToDestruct.load();

            std::vector
                <std::vector
                    <std::pair<const char*, Loggable>>> currentQueue;
            {
                std::lock_guard<std::mutex> lock(logQueueMutex);
                if(logQueue.empty())
                {
                    continue;
                }
                currentQueue.swap(logQueue);
            }

            for(auto& entry : currentQueue)
            {
                logEntry(entry);
            }

            if(onLastEntries)
            {
                return;
            }
        }
    }

    void logEntry(std::vector<std::pair<const char*, Loggable>>& entry)
    {
        streams << '{';
        for(auto it = std::begin(entry); it != std::end(entry); std::advance(it, 1))
        {
            streams << '\"' << it->first << '\"' << ':' << it->second.strValue;

            if(std::next(it) != std::end(entry))
            {
                streams << ',';
            }
        }
        streams << "}\n";
    }

    ~Logger()
    {
        waitingToDestruct.store(true);
        outputThread.join();
    }
};