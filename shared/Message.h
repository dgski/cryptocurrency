#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using byte = std::byte;

struct Message
{
    u32 id;
    u64 size;
    std::vector<byte> data;
};