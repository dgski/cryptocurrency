#include "Types.h"
#include "Communication.h"
#include "Transaction.h"
#include "Blockchain.h"

struct MSG_MANAGER_MINER_NEWBASEHASH
{
    constexpr static u32 id = 0;
    u64 newBaseHash;

    MSG_MANAGER_MINER_NEWBASEHASH(){}

    MSG_MANAGER_MINER_NEWBASEHASH(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
        parser.parse_u64(newBaseHash);
    }

    void compose(Message& msg)
    {
        msg.id = id;
        msg.compose_u64(newBaseHash);
    }
};

struct MSG_MINER_MANAGER_PROOFOFWORK
{
    constexpr static u32 id = 1;
    u64 proofOfWork;

    MSG_MINER_MANAGER_PROOFOFWORK(){}

    MSG_MINER_MANAGER_PROOFOFWORK(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
        parser.parse_u64(proofOfWork);
    }

    void compose(Message& msg)
    {
        msg.id = id;
        msg.compose_u64(proofOfWork);
    }
};

struct MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ
{
    constexpr static u32 id = 2;
    u64 numOfTransactionsRequested;

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
        parser.parse_u64(numOfTransactionsRequested);
    }

    void compose(Message& msg)
    {
        msg.id = id;
        msg.compose_u64(numOfTransactionsRequested);
    } 
};

struct MSG_A_MANAGER_TRANSACTIONER_TRANSREQ
{
    constexpr static u32 id = 3;
    std::vector<Transaction> transactions;

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {        
        u64 size;
        parser.parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& t = transactions.emplace_back();
            t.parse(parser);
        }
    }

    void compose(Message& msg)
    {
        msg.id = id;
        msg.compose_u64((u64)transactions.size());
        for(Transaction& t : transactions)
        {
            t.compose(msg);
        }
    }
};

struct MSG_CLIENT_TRANSACTIONER_NEWTRANS
{
    constexpr static u32 id = 4;
    Transaction transaction;

    MSG_CLIENT_TRANSACTIONER_NEWTRANS(){}

    MSG_CLIENT_TRANSACTIONER_NEWTRANS(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
        transaction.parse(parser);
    }

    void compose(Message& msg)
    {
        msg.id = id;
        transaction.compose(msg);
    }
};

struct MSG_MINER_MANAGER_HASHREQUEST
{
    constexpr static u32 id = 5;

    MSG_MINER_MANAGER_HASHREQUEST(){}

    MSG_MINER_MANAGER_HASHREQUEST(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
    }

    void compose(Message& msg)
    {
        msg.id = id;
    }
};

struct MSG_MANAGER_NETWORKER_NEWBLOCK
{
    constexpr static u32 id = 6;
    Block block;

    MSG_MANAGER_NETWORKER_NEWBLOCK(){}

    MSG_MANAGER_NETWORKER_NEWBLOCK(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
        block.parse(parser);
    }

    void compose(Message& msg)
    {
        msg.id = id;
        block.compose(msg);
    }
};

struct MSG_NETWORKER_NETWORKER_NEWBLOCK
{
    constexpr static u32 id = 7;

    Block block;

    MSG_NETWORKER_NETWORKER_NEWBLOCK(){}

    MSG_NETWORKER_NETWORKER_NEWBLOCK(Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser)
    {
        block.parse(parser);
    }

    void compose(Message& msg)
    {
        msg.id = id;
        block.compose(msg);
    }
};
