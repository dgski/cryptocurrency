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
    
    Loggable(str cstr)
    {
        strValue += '\"';
        strValue += cstr;
        strValue += '\"';
    }
    
    Loggable(i32 i)
    : strValue(std::to_string(i))
    {}
    
    Loggable(i64 i)
    : strValue(std::to_string(i))
    {}

    Loggable(u32 i)
    : strValue(std::to_string(i))
    {}

    Loggable(u64 i)
    : strValue(std::to_string(i))
    {}
};

class Logger;

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

    void flush()
    {
        for(std::ostream* output : outputStreams)
        {
            output->flush();
        }
    }

    friend class Logger;
};

enum class LogLevel { Info, Warning, Error };

static const char* toJSONString(LogLevel level)
{
    switch(level)
    {
        case LogLevel::Info: return "\"info\"";
        case LogLevel::Warning: return "\"warning\"";
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

    std::shared_ptr<std::thread> outputThread;
    std::atomic<bool> waitingToDestruct = false;

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

    void log(const LogLevel level, std::vector<std::pair<const char*, Loggable>>& contents)
    {
        std::lock_guard<std::mutex> lock(logQueueMutex);
        logQueue.push_back({level, getCurrentUnixTime(), std::move(contents)});
    }

    MultiStream streams;
public:

    Logger()
    {}

    void run()
    {
        outputThread = std::make_shared<std::thread>(&Logger::runOutput, this);
    }

    void logInfo(std::vector<std::pair<const char*, Loggable>> contents)
    {
        log(LogLevel::Info, contents);
    }

    void logWarning(std::vector<std::pair<const char*, Loggable>> contents)
    {
        log(LogLevel::Warning, contents);
    }

    void logError(std::vector<std::pair<const char*, Loggable>> contents)
    {
        log(LogLevel::Error, contents);
    }

    void logInfo(str entry)
    {   
        std::vector<std::pair<const char*, Loggable>> contents{{"event", entry}};
        log(LogLevel::Info, contents);
    }
    
    void logWarning(str entry)
    {   
        std::vector<std::pair<const char*, Loggable>> contents{{"event", entry}};
        log(LogLevel::Warning, contents);
    }

    void logError(str entry)
    {   
        std::vector<std::pair<const char*, Loggable>> contents{{"event", entry}};
        log(LogLevel::Error, contents);
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
        streams << '}' << '\n';
        streams.flush();
    }

    void addOutputStream(std::ostream* os)
    {
        streams.outputStreams.push_back(os);
    }

    ~Logger()
    {
        waitingToDestruct.store(true);
        outputThread->join();
    }
};

inline Logger logger;