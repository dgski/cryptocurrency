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