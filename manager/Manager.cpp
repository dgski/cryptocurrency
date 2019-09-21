#include "Manager.h"

Manager::Manager(const char* iniFileName)
{
    log("Manager module starting");
    
    const std::map<str,str> params = getInitParameters(iniFileName);

    connFromMiners.init(strToIp(params.at("connFromMiners")));
    connFromTransactioner.init(strToIp(params.at("connFromTransactioner")));
    connFromNetworker.init(strToIp(params.at("connFromNetworker")));
    registerConnections({&connFromMiners, &connFromTransactioner, &connFromNetworker});

    myPublicKey = params.at("myPublicKey");
    myPrivateKey = params.at("myPrivateKey");

    registerScheduledTask(1000, [this]()
    {
        askTransactionerForNewTransactions();
    });

    mintCurrency();
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
        case MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET::id:
        {
            processTransactionWalletInquiry(msg);
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

    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ outgoing;
    outgoing.numOfTransReq = 200 - currentBlock.transactions.size();
    
    connFromTransactioner.sendMessage(outgoing.msg(), [this](const Message& reply)
    {
        processTransactionRequestReply(reply);
    });

    registerScheduledTask(1000, [this]()
    {
        askTransactionerForNewTransactions();
    });
}

void Manager::processTransactionRequestReply(const Message& msg)
{
    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ incoming{ msg };
    
    for(Transaction& t : incoming.transactions)
    {
        addTransactionToCurrentBlock(t);
    }

    u64 newBaseHash = currentBlock.calculateBaseHash();
    if(newBaseHash != currentBaseHash)
    {
        currentBaseHash = newBaseHash;
        log("baseHash has changed, Propagating to Miners.");
        sendBaseHashToMiners();
    }

    return;
}

void Manager::addTransactionToCurrentBlock(Transaction& t)
{
    auto itSender = currentBlockWalletDeltas.find(t.sender);
    if(itSender == currentBlockWalletDeltas.end())
    {
        currentBlockWalletDeltas.emplace(t.sender, 0);
    }
    currentBlockWalletDeltas.at(t.sender) -= t.amount;

    auto itRecipiant = currentBlockWalletDeltas.find(t.recipiant);
    if(itRecipiant == currentBlockWalletDeltas.end())
    {
        currentBlockWalletDeltas.emplace(t.recipiant, 0);
    }
    currentBlockWalletDeltas.at(t.recipiant) += t.amount;

    currentBlock.transactions.push_back(t);
}

void Manager::processTransactionWalletInquiry(const Message& msg)
{
    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET incoming{ msg };

    MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY outgoing;
    auto it = wallets.find(incoming.publicWalletKey);
    outgoing.amount = (it != wallets.end()) ? it->second : 0;

    auto blockIt = currentBlockWalletDeltas.find(incoming.publicWalletKey);
    outgoing.amount +=
        (blockIt != currentBlockWalletDeltas.end()) ?
        blockIt->second :
        0;

    connFromTransactioner.sendMessage(outgoing.msg(msg.reqId));
}

void Manager::sendBaseHashToMiners()
{
    MSG_MANAGER_MINER_NEWBASEHASH outgoing;
    outgoing.newBaseHash = currentBaseHash;

    connFromMiners.sendMessage(outgoing.msg());
}

void Manager::mintCurrency()
{
    Transaction& t = currentBlock.transactions.emplace_back();
    t.amount = 2000;
    t.sender = myPublicKey;
    t.recipiant = myPublicKey;
    t.time = getCurrentUnixTime();
    t.sign(myPrivateKey, myPublicKey);

    currentBaseHash = currentBlock.calculateBaseHash();
}

void Manager::processIncomingProofOfWork(const Message& msg)
{
    MSG_MINER_MANAGER_PROOFOFWORK incoming{ msg };

    if(validProof(incoming.proofOfWork, currentBaseHash))
    {
        log("Proof is valid, Block has been mined, Sending to Networker for Propagation.");
        currentBlock.proofOfWork = incoming.proofOfWork;

        pushBlock(currentBlock);
        currentBlockWalletDeltas.clear();

        MSG_MANAGER_NETWORKER_NEWBLOCK outgoing;
        outgoing.block = currentBlock;
        connFromNetworker.sendMessage(outgoing.msg());
        
        log("Starting work on next block.");
        
        Block newBlock;
        newBlock.id = currentBlock.id + 1;
        newBlock.hashOfLastBlock = currentBlock.calculateFullHash();
        
        currentBlock = newBlock;

        mintCurrency();
        sendBaseHashToMiners();
    }
}

void Manager::processMinerHashRequest(const Message& msg)
{
    MSG_MANAGER_MINER_NEWBASEHASH outgoing;
    outgoing.newBaseHash = currentBaseHash;

    connFromMiners.sendMessage(msg.socket, outgoing.msg());
}

void Manager::processPotentialWinningBlock(const Message& msg)
{
    MSG_NETWORKER_MANAGER_NEWBLOCK incoming{ msg };

    if(currentBlock.id > incoming.block.id)
    {
        log("Block id is lower than ours; ignore");
        return;
    }

    MSG_MANAGER_NETWORKER_CHAINREQUEST outgoing;
    outgoing.maxId = incoming.block.id;
    outgoing.connId = incoming.connId;

    connFromNetworker.sendMessage(outgoing.msg(), [this](const Message& msg)
    {
        processPotentialWinningBlock_ChainReply(msg);
    });
}

void Manager::processPotentialWinningBlock_ChainReply(const Message& msg)
{
    MSG_NETWORKER_NETWORKER_CHAIN incoming{ msg };

    auto transactionHashes = getValidTransHashes(incoming.chain);
    if(!transactionHashes.has_value())
    {
        log("Chain is not valid. Aborting.");
        return;
    }
    
    log("Chain is valid. Using it from now on.");
    chain.clear();
    for(Block& block : incoming.chain)
    {
        pushBlock(block);
    }

    processPotentialWinningBlock_Finalize(transactionHashes.value());
}

void Manager::processPotentialWinningBlock_Finalize(const std::set<u64>& transactionHashes)
{   
    const Block& winningBlock = chain.back();

    currentBlock.id = winningBlock.id + 1;
    currentBlock.hashOfLastBlock = winningBlock.calculateFullHash();
    currentBlockWalletDeltas.clear();

    std::remove_if(
        std::begin(currentBlock.transactions),
        std::end(currentBlock.transactions),
        [&transactionHashes, this](Transaction& t)
        {
            return transactionHashes.count(Transaction::hashValue(t)) == 1;
        }
    );

    mintCurrency();
    sendBaseHashToMiners();
}

void Manager::processNetworkerChainRequest(const Message& msg)
{
    MSG_NETWORKER_MANAGER_CHAINREQUEST incoming{ msg };

    MSG_MANAGER_NETWORKER_CHAIN outgoing;

    std::copy_if(
        std::begin(chain),
        std::end(chain),
        std::back_inserter(outgoing.chain),
        [maxId = incoming.maxId](const Block& block)
        {
            return block.id <= maxId;
        }
    );
    
    connFromNetworker.sendMessage(outgoing.msg(msg.reqId));
}

std::optional<std::set<u64>> Manager::getValidTransHashes(std::vector<Block>& chain)
{
    std::optional<u64> hashOfLastBlock;
    std::set<u64> transactionHashes;

    for(Block& block : chain)
    {
        if(!block.isValid())
        {
            log("Block is invalid. Aborting");
            return std::nullopt;
        }

        if(!hashOfLastBlock.has_value())
        {
            hashOfLastBlock = block.calculateFullHash();
        }
        else if(block.hashOfLastBlock != hashOfLastBlock)
        {
            log("Logical error: hashOfLastBlock does not match.");
            log("Block is invalid. Aborting");
            return std::nullopt;
        }

        for(Transaction& t : block.transactions)
        {
            transactionHashes.insert(Transaction::hashValue(t));
        }
    }

    return transactionHashes;
}

void Manager::pushBlock(Block& block)
{
    for(Transaction& t : block.transactions)
    {
        if(t.sender == t.recipiant && t.amount == 2000)
        {
            wallets[t.sender] += t.amount;
            continue;
        }

        wallets[t.sender] -= t.amount;
        wallets[t.recipiant] -= t.amount;
    }

    chain.push_back(block);
}