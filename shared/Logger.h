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
    Loggable(i32 i)
    : strValue(std::to_string(i))
    {}
};

class MultiStream : public std::ostream
{
    std::vector<std::ostream*> outputStreams;

public:
    MultiStream()
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

    void addOutputStream(std::ostream* os)
    {
        outputStreams.push_back(os);
    }
};

enum class LogLevel { Info, Alert, Error };

static const char* toJSONString(LogLevel level)
{
    switch(level)
    {
        case LogLevel::Info: return "\"info\"";
        case LogLevel::Alert: return "\"alert\"";
        case LogLevel::Error: return "\"error\"";
        default: return "unmapped";
    }
}

class Logger
{
    struct WaitingLogEntry
    {
        const LogLevel level = LogLevel::Info;
        const u64 time = 0;
        const std::vector<std::pair<const char*, Loggable>> content;
    };

    std::mutex logQueueMutex;
    std::vector<WaitingLogEntry> logQueue;

    std::thread outputThread;
    std::atomic<bool> waitingToDestruct = false;

public:
    MultiStream streams;

    Logger()
    : outputThread(&Logger::runOutput, this)
    {}

    void log(const LogLevel level, std::vector<std::pair<const char*, Loggable>> contents)
    {
        const u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        std::lock_guard<std::mutex> lock(logQueueMutex);
        logQueue.push_back({level, time, std::move(contents)});
    }

    void runOutput()
    {
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            const bool onLastEntries = waitingToDestruct.load();

            std::vector<WaitingLogEntry> currentQueue;
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

    void logEntry(WaitingLogEntry& entry)
    {
        streams << '{';
        streams << "\"time\":" << entry.time << ',';
        streams << "\"level\":" << toJSONString(entry.level) << ',';
        for(auto it = std::begin(entry.content); it != std::end(entry.content); std::advance(it, 1))
        {
            streams << '\"' << it->first << '\"' << ':' << it->second.strValue;

            if(std::next(it) != std::end(entry.content))
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