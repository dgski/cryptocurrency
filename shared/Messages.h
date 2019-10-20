#include <list>

#include "Types.h"
#include "Communication.h"
#include "Transaction.h"
#include "Blockchain.h"
#include "Logger.h"

struct MSG_STRUCT
{
    virtual void parse(Parser& parser) = 0;
    virtual void compose(Message& msg) const = 0;

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

    template<u32 enforcedId>
    static void checkId(const Message& msg)
    {
        if(msg.id != enforcedId)
        {
            logger.logError({
                {"event", "Incorrect Parse Prevented"},
                {"parsingMsgId", enforcedId},
                {"givenId", msg.id}
            });
            throw std::runtime_error("Wrong message id for parsing");
        }
    }

    virtual void logMsg() const = 0;
};

struct MSG_MANAGER_MINER_NEWBASEHASH : public MSG_STRUCT
{
    constexpr static u32 id = 0;
    u64 newBaseHash;

    MSG_MANAGER_MINER_NEWBASEHASH(){}

    MSG_MANAGER_MINER_NEWBASEHASH(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(newBaseHash);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(newBaseHash);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MANAGER_MINER_NEWBASEHASH"},
            {"newBaseHash", newBaseHash}
        });
    }
};

struct MSG_MINER_MANAGER_PROOFOFWORK : public MSG_STRUCT
{
    constexpr static u32 id = 1;
    u64 proofOfWork;

    MSG_MINER_MANAGER_PROOFOFWORK(){}

    MSG_MINER_MANAGER_PROOFOFWORK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
    }

    void parse(Parser& parser) override
    {
        parser.parse(proofOfWork);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(proofOfWork);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MINER_MANAGER_PROOFOFWORK"},
            {"proofOfWork", proofOfWork}
        });
    }
};

struct MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ : public MSG_STRUCT
{
    constexpr static u32 id = 2;
    u64 numOfTransReq;

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(numOfTransReq);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(numOfTransReq);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ"},
            {"numOfTransReq", numOfTransReq}
        });
    }
};

struct MSG_A_MANAGER_TRANSACTIONER_TRANSREQ : public MSG_STRUCT
{
    constexpr static u32 id = 3;
    std::list<Transaction> transactions;

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(transactions);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(transactions);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_A_MANAGER_TRANSACTIONER_TRANSREQ"},
            {"transactions.size()", (u64)transactions.size()}
        });
    }
};

struct MSG_CLIENT_TRANSACTIONER_NEWTRANS : public MSG_STRUCT
{
    constexpr static u32 id = 4;
    Transaction transaction;

    MSG_CLIENT_TRANSACTIONER_NEWTRANS(){}

    MSG_CLIENT_TRANSACTIONER_NEWTRANS(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(transaction);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(transaction);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_CLIENT_TRANSACTIONER_NEWTRANS"},
            {"transaction.time", transaction.time},
            {"transaction.amount", transaction.amount}
        });
    }
};

struct MSG_MINER_MANAGER_HASHREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 5;

    MSG_MINER_MANAGER_HASHREQUEST(){}

    MSG_MINER_MANAGER_HASHREQUEST(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MINER_MANAGER_HASHREQUEST"},
        });
    }
};

struct MSG_MANAGER_NETWORKER_NEWBLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 6;
    Block block;

    MSG_MANAGER_NETWORKER_NEWBLOCK(){}

    MSG_MANAGER_NETWORKER_NEWBLOCK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MANAGER_NETWORKER_NEWBLOCK"},
            {"block.transactions.size()", (u64)block.transactions.size()},
            {"block.proofOfWork", block.proofOfWork}
        });
    }
};

struct MSG_NETWORKER_NETWORKER_NEWBLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 7;

    Block block;

    MSG_NETWORKER_NETWORKER_NEWBLOCK(){}

    MSG_NETWORKER_NETWORKER_NEWBLOCK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_NETWORKER_NEWBLOCK"},
            {"block.transactions.size()", (u64)block.transactions.size()},
            {"block.proofOfWork", block.proofOfWork}
        });
    }
};

struct MSG_NETWORKER_NETWORKER_REGISTERME : public MSG_STRUCT
{
    constexpr static u32 id = 8;
    
    str connStr;

    MSG_NETWORKER_NETWORKER_REGISTERME(){}

    MSG_NETWORKER_NETWORKER_REGISTERME(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(connStr);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(connStr);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_NETWORKER_REGISTERME"},
            {"connStr", connStr}
        });
    }
};

struct MSG_NETWORKER_MANAGER_NEWBLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 9;

    Block block;

    MSG_NETWORKER_MANAGER_NEWBLOCK(){}

    MSG_NETWORKER_MANAGER_NEWBLOCK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_MANAGER_NEWBLOCK"},
            {"block.id", block.id},
            {"block.transactions.size()", (u64)block.transactions.size()},
            {"block.proofOfWork", block.proofOfWork},
        });
    }
};

struct MSG_MANAGER_NETWORKER_BLOCKREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 10;

    bool voidRequest;
    u64 blockId;

    MSG_MANAGER_NETWORKER_BLOCKREQUEST(){}

    MSG_MANAGER_NETWORKER_BLOCKREQUEST(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(blockId, voidRequest);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(blockId, voidRequest);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MANAGER_NETWORKER_BLOCKREQUEST"},
            {"blockId", blockId},
            {"voidRequest", voidRequest}
        });
    }
};

struct MSG_NETWORKER_NETWORKER_BLOCKREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 11;

    u64 blockId;

    MSG_NETWORKER_NETWORKER_BLOCKREQUEST(){}

    MSG_NETWORKER_NETWORKER_BLOCKREQUEST(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(blockId);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(blockId);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_NETWORKER_BLOCKREQUEST"},
            {"blockId", blockId}
        });
    }
};

struct MSG_NETWORKER_MANAGER_BLOCKREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 11;

    u64 blockId;

    MSG_NETWORKER_MANAGER_BLOCKREQUEST(){}

    MSG_NETWORKER_MANAGER_BLOCKREQUEST(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(blockId);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(blockId);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_MANAGER_BLOCKREQUEST"},
            {"blockId", blockId}
        });
    }
};

struct MSG_MANAGER_NETWORKER_BLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 12;

    Block block;

    MSG_MANAGER_NETWORKER_BLOCK(){}

    MSG_MANAGER_NETWORKER_BLOCK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MANAGER_NETWORKER_BLOCK"},
            {"block.id", block.id},
            {"block.transactions.size()", (u64)block.transactions.size()},
            {"block.proofOfWork", block.proofOfWork}
        });
    }
};

struct MSG_NETWORKER_MANAGER_BLOCK : public MSG_STRUCT
{
    constexpr static u32 id = 13;

    Block block;

    MSG_NETWORKER_MANAGER_BLOCK(){}

    MSG_NETWORKER_MANAGER_BLOCK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_MANAGER_BLOCK"},
            {"block.id", block.id},
            {"block.transactions.size()", (u64)block.transactions.size()},
            {"block.proofOfWork", block.proofOfWork}
        });
    }
};

struct MSG_NETWORKER_NETWORKER_BLOCK: public MSG_STRUCT
{
    constexpr static u32 id = 14;

    Block block;

    MSG_NETWORKER_NETWORKER_BLOCK(){}

    MSG_NETWORKER_NETWORKER_BLOCK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_NETWORKER_NETWORKER_BLOCK"},
            {"block.id", block.id},
            {"block.transactions.size()", (u64)block.transactions.size()},
            {"block.proofOfWork", block.proofOfWork}
        });
    }
};

struct MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET : public MSG_STRUCT
{
    constexpr static u32 id = 15;
    
    str publicWalletKey;

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET(){}

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(publicWalletKey);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(publicWalletKey);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET"},
            {"hash(publicWalletKey)", (u64)std::hash<str>{}(publicWalletKey)}
        });
    }
};

struct MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY : public MSG_STRUCT
{
    constexpr static u32 id = 16;
    
    u64 amount = 0;

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY(){}

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(amount);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(amount);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY"},
            {"amount", amount}
        });
    }
};

struct MSG_MODULE_LOGCOLLECTOR_LOGREADY : public MSG_STRUCT
{
    constexpr static u32 id = 17;

    str name;
    
    MSG_MODULE_LOGCOLLECTOR_LOGREADY(){}

    MSG_MODULE_LOGCOLLECTOR_LOGREADY(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(name);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(name);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MODULE_LOGCOLLECTOR_LOGSREADY"},
            {"name", name}
        });
    }
};

struct MSG_LOGCOLLECTOR_MODULE_LOGREQUEST : public MSG_STRUCT
{
    constexpr static u32 id = 18;
    
    MSG_LOGCOLLECTOR_MODULE_LOGREQUEST(){}

    MSG_LOGCOLLECTOR_MODULE_LOGREQUEST(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {}

    void compose(Message& msg) const override
    {
        msg.id = id;
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_LOGCOLLECTOR_MODULE_LOGREQUEST"},
        });
    }
};

struct MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK : public MSG_STRUCT
{
    constexpr static u32 id = 19;

    str log;
    
    MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK(){}

    MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(log);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(log);
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK"},
            {"log.size()", (u64)log.size()}
        });
    }
};

struct MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK : public MSG_STRUCT
{
    constexpr static u32 id = 20;
    
    MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK(){}

    MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK(const Message& msg)
    {
        checkId<id>(msg);
        Parser parser(msg);
        parse(parser);
        logMsg();
    }

    void parse(Parser& parser) override
    {}

    void compose(Message& msg) const override
    {
        msg.id = id;
    }

    void logMsg() const override
    {
        logger.logInfo({
            {"event", "message"},
            {"name", "MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK"},
        });
    }
};