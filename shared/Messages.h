#include "Types.h"
#include "Communication.h"
#include "Transaction.h"
#include "Blockchain.h"

struct MSG_STRUCT
{
    virtual void parse(Parser& parser) = 0;
    virtual void compose(Message& msg) = 0;

    Message msg()
    {
        Message msg;
        compose(msg);
        return msg;
    }

    Message msg(u32 reqId)
    {
        Message msg;
        msg.reqId = reqId;
        compose(msg);
        return msg;
    }
};

struct MSG_MANAGER_MINER_NEWBASEHASH : public MSG_STRUCT
{
    constexpr static u32 id = 0;
    u64 newBaseHash;

    MSG_MANAGER_MINER_NEWBASEHASH(){}

    MSG_MANAGER_MINER_NEWBASEHASH(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse_u64(newBaseHash);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64(newBaseHash);
    }

    void logMsg() const
    {
        log(
            "MSG_MANAGER_MINER_NEWBASEHASH{ newBaseHash: % }",
            newBaseHash
        );
    }
};

struct MSG_MINER_MANAGER_PROOFOFWORK : public MSG_STRUCT
{
    constexpr static u32 id = 1;
    u64 proofOfWork;

    MSG_MINER_MANAGER_PROOFOFWORK(){}

    MSG_MINER_MANAGER_PROOFOFWORK(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {
        parser.parse_u64(proofOfWork);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64(proofOfWork);
    }
};

struct MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ : public MSG_STRUCT
{
    constexpr static u32 id = 2;
    u64 numOfTransactionsRequested;

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse_u64(numOfTransactionsRequested);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64(numOfTransactionsRequested);
    }

    void logMsg() const
    {
        log(
            "MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ numOfTransactionsRequested=%",
            numOfTransactionsRequested
        );
    }
};

struct MSG_A_MANAGER_TRANSACTIONER_TRANSREQ : public MSG_STRUCT
{
    constexpr static u32 id = 3;
    std::vector<Transaction> transactions;

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {        
        u64 size;
        parser.parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& t = transactions.emplace_back();
            t.parse(parser);
        }
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64((u64)transactions.size());
        for(Transaction& t : transactions)
        {
            t.compose(msg);
        }
    }
};

struct MSG_CLIENT_TRANSACTIONER_NEWTRANS : public MSG_STRUCT
{
    constexpr static u32 id = 4;
    Transaction transaction;

    MSG_CLIENT_TRANSACTIONER_NEWTRANS(){}

    MSG_CLIENT_TRANSACTIONER_NEWTRANS(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        transaction.parse(parser);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        transaction.compose(msg);
    }

    void logMsg() const
    {
        log("MSG_CLIENT_TRANSACTIONER_NEWTRANS");
        transaction.logTransaction();
    }
};

struct MSG_MINER_MANAGER_HASHREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 5;

    MSG_MINER_MANAGER_HASHREQUEST(){}

    MSG_MINER_MANAGER_HASHREQUEST(const Message& msg)
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

struct MSG_MANAGER_NETWORKER_NEWBLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 6;
    Block block;

    MSG_MANAGER_NETWORKER_NEWBLOCK(){}

    MSG_MANAGER_NETWORKER_NEWBLOCK(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        block.parse(parser);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        block.compose(msg);
    }

    void logMsg()
    {
        log(
            "MSG_MANAGER_NETWORKER_NEWBLOCK"
            "{ numOfTrans:%, proofOfWork:% }",
            block.transactions.size(),
            block.proofOfWork
        );
    }
};

struct MSG_NETWORKER_NETWORKER_NEWBLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 7;

    Block block;

    MSG_NETWORKER_NETWORKER_NEWBLOCK(){}

    MSG_NETWORKER_NETWORKER_NEWBLOCK(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        block.parse(parser);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        block.compose(msg);
    }

    void logMsg()
    {
        log(
            "MSG_NETWORKER_NETWORKER_NEWBLOCK"
            "{ numOfTrans:%, proofOfWork:% }",
            block.transactions.size(),
            block.proofOfWork
        );
    }
};

struct MSG_NETWORKER_NETWORKER_REGISTERME : public MSG_STRUCT
{
    constexpr static u32 id = 8;
    
    str connStr;

    MSG_NETWORKER_NETWORKER_REGISTERME(){}

    MSG_NETWORKER_NETWORKER_REGISTERME(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse_str(connStr);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_str(connStr);
    }

    void logMsg()
    {
        log(
            "MSG_NETWORKER_NETWORKER_REGISTERME"
            "{ connStr:% }",
            connStr
        );
    }
};

struct MSG_NETWORKER_MANAGER_NEWBLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 9;

    Block block;
    i32 connId;

    MSG_NETWORKER_MANAGER_NEWBLOCK(){}

    MSG_NETWORKER_MANAGER_NEWBLOCK(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {
        block.parse(parser);
        parser.parser_i32(connId);
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        block.compose(msg);
        msg.compose_i32(connId);
    }
};

struct MSG_MANAGER_NETWORKER_CHAINREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 10;

    u64 maxId;
    i32 connId;

    MSG_MANAGER_NETWORKER_CHAINREQUEST(){}

    MSG_MANAGER_NETWORKER_CHAINREQUEST(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser)
    {        
        parser.parse_u64(maxId);
    }

    void compose(Message& msg)
    {
        msg.compose_u64(maxId);
    }

    void logMsg()
    {
        log(
            "MSG_MANAGER_NETWORKER_CHAINREQUEST"
            "{ maxId:%, connId:% }",
            maxId,
            connId
        );
    }
};

struct MSG_NETWORKER_NETWORKER_CHAINREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 11;

    u64 maxId;

    MSG_NETWORKER_NETWORKER_CHAINREQUEST(){}

    MSG_NETWORKER_NETWORKER_CHAINREQUEST(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {        
        parser.parse_u64(maxId);
    }

    void compose(Message& msg) override
    {
        msg.compose_u64(maxId);
    }
};

struct MSG_NETWORKER_MANAGER_CHAINREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 11;

    u64 maxId;

    MSG_NETWORKER_MANAGER_CHAINREQUEST(){}

    MSG_NETWORKER_MANAGER_CHAINREQUEST(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {        
        parser.parse_u64(maxId);
    }

    void compose(Message& msg) override
    {
        msg.compose_u64(maxId);
    }
};

struct MSG_MANAGER_NETWORKER_CHAIN : public MSG_STRUCT
{
    constexpr static u32 id = 12;

    std::vector<Block> chain;

    MSG_MANAGER_NETWORKER_CHAIN(){}

    MSG_MANAGER_NETWORKER_CHAIN(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {        
        u64 size;
        parser.parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& b = chain.emplace_back();
            b.parse(parser);
        }
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64((u64)chain.size());
        for(Block& b : chain)
        {
            b.compose(msg);
        }
    }
};

struct MSG_NETWORKER_MANAGER_CHAIN : public MSG_STRUCT
{
    constexpr static u32 id = 13;

    std::vector<Block> chain;

    MSG_NETWORKER_MANAGER_CHAIN(){}

    MSG_NETWORKER_MANAGER_CHAIN(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {        
        u64 size;
        parser.parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& b = chain.emplace_back();
            b.parse(parser);
        }
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64((u64)chain.size());
        for(Block& b : chain)
        {
            b.compose(msg);
        }
    }
};

struct MSG_NETWORKER_NETWORKER_CHAIN : public MSG_STRUCT
{
    constexpr static u32 id = 14;

    std::vector<Block> chain;

    MSG_NETWORKER_NETWORKER_CHAIN(){}

    MSG_NETWORKER_NETWORKER_CHAIN(const Message& msg)
    {
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {        
        u64 size;
        parser.parse_u64(size);
        for(u64 i{ 0 }; i < size; ++i)
        {
            auto& b = chain.emplace_back();
            b.parse(parser);
        }
    }

    void compose(Message& msg) override
    {
        msg.id = id;
        msg.compose_u64((u64)chain.size());
        for(Block& b : chain)
        {
            b.compose(msg);
        }
    }
};