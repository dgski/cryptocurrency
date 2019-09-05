#include "Manager.h"

Manager::Manager(const char* iniFileName)
{
    log("Manager module starting");
    
    const std::map<str,str> params = getInitParameters(iniFileName);

    connFromMiners.init(strToIp(params.at("connFromMiners")));
    registerServerConnection(&connFromMiners);

    connFromTransactioner.init(strToIp(params.at("connFromTransactioner")));
    registerServerConnection(&connFromTransactioner);

    connFromNetworker.init(strToIp(params.at("connFromNetworker")));
    registerServerConnection(&connFromNetworker);

    registerScheduledTask(
        1000,
        [this]()
        {
            askTransactionerForNewTransactions();
        }
    );

    currentBaseHash = 12393939334343; // FAKE
}

void Manager::processMessage(const Message& msg)
{
    switch(msg.id)
    {
        case MSG_MINER_MANAGER_PROOFOFWORK::id:
        {
            processIncomingProofOfWork(msg);
            return;
        }
        case MSG_MINER_MANAGER_HASHREQUEST::id:
        {
            processMinerHashRequest(msg);
            return;
        }
        case MSG_NETWORKER_MANAGER_NEWBLOCK::id:
        {
            processPotentialWinningBlock(msg);
            return;
        }
        case MSG_NETWORKER_MANAGER_CHAINREQUEST::id:
        {
            processNetworkerChainRequest(msg);
            return;
        }
        default:
        {
            log("Unhandled MSG id=%", msg.id);
            return;
        }
    }
}

void Manager::askTransactionerForNewTransactions()
{   
    if(currentBlock.transactions.size() == 200)
    {
        return;
    }

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents;
    contents.numOfTransactionsRequested = 200 - currentBlock.transactions.size();
    connFromTransactioner.sendMessage(contents.msg(), [this](const Message& reply)
    {
        processTransactionRequestReply(reply);
    });

    registerScheduledTask(
        1000,
        [this]()
        {
            askTransactionerForNewTransactions();
        }
    );
}

void Manager::processTransactionRequestReply(const Message& msg)
{
    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };

    log(
        "processTransactionRequestReply recieved % transactions",
        contents.transactions.size()
    );
    
    std::move(
        std::begin(contents.transactions),
        std::end(contents.transactions),
        std::back_inserter(currentBlock.transactions)
    );

    u64 newBaseHash = currentBlock.calculateBaseHash();
    if(newBaseHash != currentBaseHash)
    {
        currentBaseHash = newBaseHash;
        log("baseHash has changed, Propagating to Miners.");
        sendBaseHashToMiners();
    }

    return;
}

void Manager::sendBaseHashToMiners()
{
    MSG_MANAGER_MINER_NEWBASEHASH contents;
    contents.newBaseHash = currentBaseHash;

    connFromMiners.sendMessage(contents.msg());
}

void Manager::mintCurrency()
{
    Transaction t;
    t.amount = 2000;
    t.sender = myPublicKey;
    t.recipiant = myPublicKey;
    t.time = getCurrentUnixTime();
    t.signature = "signature";

    currentBlock.transactions.push_back(t);
}

void Manager::processIncomingProofOfWork(const Message& msg)
{
    MSG_MINER_MANAGER_PROOFOFWORK contents{ msg };
    log("MSG_MINER_MANAGER_PROOFOFWORK proof=%", contents.proofOfWork);

    if(validProof(contents.proofOfWork, currentBaseHash))
    {
        log("Proof is valid, Block has been mined, Sending to Networker for Propagation.");
        currentBlock.proofOfWork = contents.proofOfWork;

        chain.push_back(currentBlock);

        MSG_MANAGER_NETWORKER_NEWBLOCK blockContents;
        blockContents.block = currentBlock;
        connFromNetworker.sendMessage(blockContents.msg());
        
        log("Starting work on next block.");
        
        Block newBlock;
        newBlock.id = currentBlock.id + 1;
        newBlock.hashOfLastBlock = currentBlock.calculateFullHash();
        
        currentBlock = newBlock;
        mintCurrency();
        currentBaseHash = currentBlock.calculateBaseHash();

        sendBaseHashToMiners();
    }
}

void Manager::processMinerHashRequest(const Message& msg)
{
    log("MSG_MINER_MANAGER_HASHREQUEST");

    MSG_MANAGER_MINER_NEWBASEHASH contents;
    contents.newBaseHash = currentBaseHash;
    Message hashMsg;
    contents.compose(hashMsg);
    connFromMiners.sendMessage(msg.socket, hashMsg);
}

void Manager::processPotentialWinningBlock(const Message& msg)
{
    MSG_NETWORKER_MANAGER_NEWBLOCK contents{ msg };
    log("Received Potential New Winning Block");

    if(currentBlock.id > contents.block.id)
    {
        log("Block id is lower than ours; ignore");
        return;
    }

    MSG_MANAGER_NETWORKER_CHAINREQUEST reply;
    reply.maxId = contents.block.id;
    reply.connId = contents.connId;

    connFromNetworker.sendMessage(reply.msg(), [this](const Message& msg)
    {
        processPotentialWinningBlock_ChainReply(msg);
    });
}

void Manager::processPotentialWinningBlock_ChainReply(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_CHAIN contents{ msg };
    log(
        "Received potential replacement chain, length: %",
        contents.chain.size()
    );

    std::optional<u64> hashOfLastBlock;
    std::set<u64> transactionHashes;

    for(Block& block : contents.chain)
    {
        if(!block.isValid())
        {
            log("Chain is invalid. Aborting");
            return;
        }

        if(!hashOfLastBlock.has_value())
        {
            hashOfLastBlock = block.calculateFullHash();
        }
        else if(block.hashOfLastBlock != hashOfLastBlock)
        {
            log("Logical error: hashOfLastBlock does not match.");
            log("Chain is invalid. Aborting");
            return;
        }

        for(Transaction& t : block.transactions)
        {
            transactionHashes.insert(std::hash<Transaction>{}(t));
        }
    }

    chain = std::move(contents.chain);
}

void Manager::processPotentialWinningBlock_Finalize(const std::set<u64>& transactionHashes)
{   
    const Block& winningBlock = chain.back();

    currentBlock.id = winningBlock.id + 1;
    currentBlock.hashOfLastBlock = winningBlock.calculateFullHash();

    std::remove_if(
        std::begin(currentBlock.transactions),
        std::end(currentBlock.transactions),
        [&transactionHashes, this](Transaction& t)
        {
            return transactionHashes.count(std::hash<Transaction>{}(t)) == 1;
        }
    );

    mintCurrency();
    currentBaseHash = currentBlock.calculateBaseHash();
    sendBaseHashToMiners();
}

void Manager::processNetworkerChainRequest(const Message& msg)
{
    MSG_NETWORKER_MANAGER_CHAINREQUEST contents{ msg };

    MSG_MANAGER_NETWORKER_CHAIN reply;

    std::copy_if(
        std::begin(chain),
        std::end(chain),
        std::back_inserter(reply.chain),
        [&contents](const Block& block)
        {
            return block.id <= contents.maxId;
        }
    );
    
    connFromNetworker.sendMessage(reply.msg(msg.reqId));
}