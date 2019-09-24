#include <initializer_list>
#include <tuple>
#include <variant>
#include <iostream>
#include <sstream>
#include <vector>

#include "Types.h"

struct Logger
{
    std::vector<std::ostream*> streams;

    template<typename T>
    void push(T text)
    {
        for(auto s : streams)
        {
            *s << text;
        }
    }
};

using Loggable = std::variant<str, u32, u64, i32, i64>;

struct StreamOut
{
    std::ostream& os;

    StreamOut(std::ostream& _os)
    : os(_os)
    {}

    template<typename T>
    void operator()(const T& val)
    {
        os << val;
    }

    void operator()(const str& val)
    {
        os << '\"' << val << '\"';
    }
};

/*
void logPlus(std::initializer_list<std::pair<str, Loggable>> logList)
{
    std::cout << "{";
    for(auto it = logList.begin(); it != logList.end(); std::advance(it, 1))
    {
        std::cout << it->first;
        std::cout << ":";
        std::visit(StreamOut{}, it->second);
        if(std::next(it) != logList.end())
        {
            std::cout << ",";
        }
    }
    std::cout << "}\n";
}
*/

void toJSONStream(std::ostream& stream, std::initializer_list<std::pair<str, Loggable>>& logList)
{
    stream << '{';
    for(auto it = logList.begin(); it != logList.end(); std::advance(it, 1))
    {
        stream << it->first;
        stream << ':';
        std::visit(StreamOut{stream}, it->second);
        if(std::next(it) != logList.end())
        {
            stream << ',';
        }
    }
    stream << "}\n";
}

str toJSONStr(std::initializer_list<std::pair<str, Loggable>> logList)
{
    std::stringstream ss;
    toJSONStream(ss, logList);
    return ss.str();
}