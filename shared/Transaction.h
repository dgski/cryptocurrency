#pragma once

#include "Types.h"
#include "Communication.h"

struct Transaction
{
    u64 id;

    void parse(Parser& parser)
    {
        parser.parse_u64(id);
    }

    void compose(Message& msg)
    {
        msg.compose_u64(id);
    }
};

namespace std
{
    template<>
    struct hash<Transaction>
    {
        typedef Transaction& argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& transaction) const noexcept
        {
            return std::hash<int>{}(transaction.id);
        }
    };
}