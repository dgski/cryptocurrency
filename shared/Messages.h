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

    virtual void logMsg() const = 0;
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
        log(
            "MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ"
            "{ numOfTransReq:% }",
            numOfTransReq
        );
    }
};

struct MSG_A_MANAGER_TRANSACTIONER_TRANSREQ : public MSG_STRUCT
{
    constexpr static u32 id = 3;
    std::list<Transaction> transactions;

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(){}

    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ(const Message& msg)
    {
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
        log(
            "MSG_A_MANAGER_TRANSACTIONER_TRANSREQ"
            "{ transactions:% }",
            transactions.size()
        );
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
        parser.parse(transaction);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(transaction);
    }

    void logMsg() const override
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
        log(
            "MSG_MINER_MANAGER_HASHREQUEST"
            "{}"
        );
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
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
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
        parser.parse(block);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block);
    }

    void logMsg() const override
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
        parser.parse(connStr);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(connStr);
    }

    void logMsg() const override
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
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(block, connId);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(block, connId);
    }

    void logMsg() const override
    {
        log(
            "MSG_NETWORKER_MANAGER_NEWBLOCK"
            "{ block:{}, connId:% }",
            connId
        );
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

    void parse(Parser& parser) override
    {        
        parser.parse(maxId);
    }

    void compose(Message& msg) const override
    {
        msg.compose(maxId);
    }

    void logMsg() const override
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
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(maxId);
    }

    void compose(Message& msg) const override
    {
        msg.compose(maxId);
    }

    void logMsg() const override
    {
        log(
            "MSG_NETWORKER_NETWORKER_CHAINREQUEST"
            "{ maxId:% }",
            maxId
        );
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
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(maxId);
    }

    void compose(Message& msg) const override
    {
        msg.compose(maxId);
    }

    void logMsg() const override
    {
        log(
            "MSG_NETWORKER_MANAGER_CHAINREQUEST"
            "{ maxId:% }",
            maxId
        );
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
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(chain);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(chain);
    }

    void logMsg() const override
    {
        log(
            "MSG_MANAGER_NETWORKER_CHAIN"
            "{ chain.size:% }",
            chain.size()
        );
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
        logMsg();
    }

    void parse(Parser& parser) override
    {
        parser.parse(chain);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(chain);
    }

    void logMsg() const override
    {
        log(
            "MSG_NETWORKER_MANAGER_CHAIN"
            "{ chain.size:% }",
            chain.size()
        );
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
        logMsg();
    }

    void parse(Parser& parser) override
    {        
        parser.parse(chain);
    }

    void compose(Message& msg) const override
    {
        msg.id = id;
        msg.compose(chain);
    }

    void logMsg() const override
    {
        log(
            "MSG_NETWORKER_NETWORKER_CHAIN"
            "{ chain.size:% }",
            chain.size()
        );
    }
};

struct MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET : public MSG_STRUCT
{
    constexpr static u32 id = 15;
    
    str publicWalletKey;

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET(){}

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET(const Message& msg)
    {
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
        log(
          "MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET"
          "{ publicWalletKey.lenght():% }",
          publicWalletKey.length()
        );
    }
};

struct MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY : public MSG_STRUCT
{
    constexpr static u32 id = 16;
    
    u64 amount = 0;

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY(){}

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY(const Message& msg)
    {
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
        log(
          "MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET"
          "{ amount:% }",
          amount
        );
    }
};