#include <iostream>
#include <fstream>
#include <filesystem>

#include "Utils.h"

std::pair<str,str> getLineKeyValuePair(const str& line)
{
    std::pair<str,str> result;

    str* dest = &(std::get<0>(result));
    bool keyGrabbed = false;

    for(char c : line)
    {
        if(c == '=' && keyGrabbed == false)
        {
            keyGrabbed = true;
            dest = &(std::get<1>(result));
        }
        else
        {
            *dest += c;
        }
    }

    return result;
}

std::map<str, str> getInitParameters(const char* fileName)
{
    std::map<str,str> res;
    std::ifstream iniFile{ fileName };

    str line;
    while(getline(iniFile, line))
    {
        res.insert(getLineKeyValuePair(line));
    }

    return res;
}

bool validProof(u64 nonce, u64 hash)
{
    const size_t res = std::hash<u64>{}(nonce) ^ std::hash<u64>{}(hash);
    const size_t mask = (size_t)(0b111111111111111111111111111111);
    return ((mask ^ res) & mask) == mask;
}

IpInfo strToIp(str s)
{
    IpInfo res;

    auto it = std::find(s.begin(), s.end(), ':');
    std::copy(s.begin(), it, std::back_inserter(res.address));
    res.port = atoi(str(std::next(it), s.end()).c_str());
    
    return res;
}

std::vector<str> splitStr(const str& input)
{
    if(input.empty())
    {
        return {};
    }

    std::vector<str> output;
    std::string tmp;
    for(char c : input)
    {   
        if(c == ',')
        {
            output.push_back(std::move(tmp));
            tmp.clear();
            continue;
        }
        tmp.push_back(c);
    }

    output.push_back(std::move(tmp));
    return output;
}

u64 getCurrentUnixTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}