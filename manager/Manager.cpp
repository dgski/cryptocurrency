#include "Manager.h"

Manager::Manager(const char* iniFileName) : Module("manager")
{
    const std::map<str,str> params = getInitParameters(iniFileName);
    init(params);
    
    logger.logInfo("Manager module starting");
    
    connFromMiners.init(strToIp(params.at("connFromMiners")));
    connFromTransactioner.init(strToIp(params.at("connFromTransactioner")));
    connFromNetworker.init(strToIp(params.at("connFromNetworker")));
    registerConnections({&connFromMiners, &connFromTransactioner, &connFromNetworker});
    
    walletKeys = RSAKeyPair::create(
        params.at("privateKeyFile"),
        params.at("publicKeyFile")
    );
    if(!walletKeys.has_value())
    {
        throw std::runtime_error("Wallet Keypair could not be read.");
    }

    registerScheduledTask(ASK_FOR_TRANS_FREQ, [this]()
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
        case MSG_NETWORKER_MANAGER_BLOCKREQUEST::id:
        {
            processNetworkerBlockRequest(msg);
            return;
        }
        case MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET::id:
        {
            processTransactionWalletInquiry(msg);
            return;
        }
        default:
        {
            processUnhandledMessage(msg);
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

    registerScheduledTask(ASK_FOR_TRANS_FREQ, [this]()
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
        logger.logInfo({
            {"event", "currentBaseHash has changed, Propagating to Miners."},
            {"currentBaseHash", currentBaseHash}   
        });
        sendBaseHashToMiners();
    }

    return;
}

void Manager::addTransactionToCurrentBlock(Transaction& t)
{
    addTransactionToWallets(currentBlockWalletDeltas, t);
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
    t.sender = walletKeys.value().publicKey;
    t.recipiant = walletKeys.value().publicKey;
    t.time = getCurrentUnixTime();
    t.sign(walletKeys.value());

    currentBaseHash = currentBlock.calculateBaseHash();
}

void Manager::processIncomingProofOfWork(const Message& msg)
{
    MSG_MINER_MANAGER_PROOFOFWORK incoming{ msg };

    if(validProof(incoming.proofOfWork, currentBaseHash))
    {
        logger.logInfo({
            {"event", "Proof valid, Block mined, Propagating."},
            {"currentBaseHash", currentBaseHash},
            {"proofOfWork", incoming.proofOfWork}
        });

        currentBlock.proofOfWork = incoming.proofOfWork;

        pushBlock(currentBlock);
        currentBlockWalletDeltas.clear();

        MSG_MANAGER_NETWORKER_NEWBLOCK outgoing;
        outgoing.block = currentBlock;
        connFromNetworker.sendMessage(outgoing.msg());
        
        logger.logInfo("Starting work on next block.");
        
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

    if(chainValidationCapacity == 0)
    {
        logger.logInfo("chainValidationCapacity == 0; Cannot validate another.");

        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(msg.reqId));

        return;
    }

    if(currentBlock.id >= incoming.block.id)
    {
        logger.logInfo({
            {"event", "Block id is lower or equal to ours; ignore"},
            {"currentBlock.id", currentBlock.id},
            {"incoming.block.id", incoming.block.id}
        });
        
        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(msg.reqId));

        return;
    }

    chainValidationCapacity -= 1;

    std::list<Block> potentialChain{ std::move(incoming.block) };
    tryAbsorbChain(msg.reqId, std::move(potentialChain));
}

void Manager::tryAbsorbChain(u32 reqId, std::list<Block> potentialChain)
{
    logger.logInfo({
        {"potentialChain.size()", (u64)potentialChain.size()}
    });

    Block& frontBlock = potentialChain.front();

    if(!frontBlock.isValid())
    {
        logger.logInfo({
            {"event", "Block is not valid; not requesting further blocks"}
        });

        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(reqId));
        
        chainValidationCapacity += 1;
        return;
    }

    if(frontBlock.id == 0 || hashToBlock.find(frontBlock.calculateFullHash()) != std::end(hashToBlock))
    {
        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(reqId));

        finalizeAbsorbChain(std::move(potentialChain));
        chainValidationCapacity += 1;
        return;
    }

    MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
    outgoing.voidRequest = false;
    outgoing.blockId = potentialChain.front().id - 1;
    
    connFromNetworker.sendMessage(
        outgoing.msg(reqId),
        [this, chain = std::move(potentialChain)](const Message& msg)
        {
            MSG_NETWORKER_MANAGER_BLOCK incoming( msg );

            std::list<Block> potentialChain = std::move(chain);
            potentialChain.push_front(incoming.block);
            
            tryAbsorbChain(msg.reqId, std::move(potentialChain));
        });
}

void Manager::finalizeAbsorbChain(std::list<Block> potentialChainFragment)
{
    if(chain.back().id >= potentialChainFragment.back().id)
    {
        logger.logInfo("In the meantime, our chain has become bigger, discarding fragment");
        return;
    }

    logger.logInfo("Foreign chain valid. Absorbing.");

    Block& frontBlock = potentialChainFragment.front();

    // Find start of current chain fragment that will be replaced
    auto itBlockIt = hashToBlock.find(frontBlock.calculateFullHash());

    auto itToRemoveBegin = (itBlockIt != std::end(hashToBlock))?
        itBlockIt->second :
        std::begin(chain);

    // Remove effects of removed blocks
    std::for_each(itToRemoveBegin, std::end(chain), [this](const Block& block)
    {
        for(const Transaction& t : block.transactions)
        {
            removeTransactionFromWallets(wallets, t);
        }

        hashToBlock.erase(block.calculateFullHash());
        idToBlock.erase(block.id);
    });
    
    // Add effects of added blocks
    for(auto it = std::begin(potentialChainFragment); it != std::end(potentialChainFragment); ++it)
    {
        for(const Transaction& t : it->transactions)
        {
            addTransactionToWallets(wallets, t);
            removeTransactionFromCurrentBlock(t);
        }

        hashToBlock.emplace(it->calculateFullHash(), it);
        idToBlock.emplace(it->id, it);
    }

    // Splice the incoming fragment onto the end of the chain
    chain.erase(itToRemoveBegin, std::cend(chain));
    chain.splice(std::cend(chain), potentialChainFragment);

    // Start working on new Block
    currentBlock.id = chain.back().id + 1;
    currentBlock.hashOfLastBlock = chain.back().calculateFullHash();

    mintCurrency();
    sendBaseHashToMiners();
}

void Manager::processNetworkerBlockRequest(const Message& msg)
{
    MSG_NETWORKER_MANAGER_BLOCKREQUEST incoming{ msg };

    const auto it = idToBlock.find(incoming.blockId);
    if(it != std::end(idToBlock))
    {
        MSG_MANAGER_NETWORKER_BLOCK outgoing;
        outgoing.block = *it->second;
        connFromNetworker.sendMessage(outgoing.msg(msg.reqId));
    }    
}

void Manager::pushBlock(Block& block)
{
    for(Transaction& t : block.transactions)
    {
        addTransactionToWallets(wallets, t);
    }

    chain.push_back(block);
    
    hashToBlock.emplace(block.calculateFullHash(), std::prev(chain.end()));
    idToBlock.emplace(block.id, std::prev(chain.end()));
}

void Manager::addTransactionToWallets(std::map<str, i64>& wallets, const Transaction& t)
{
    auto itSender = wallets.find(t.sender);

    if(t.sender == t.recipiant && t.amount == 2000)
    {
        addToMapElementOrInsertZero(wallets, t.sender, (i64)t.amount);
        return;
    }

    addToMapElementOrInsertZero(wallets, t.sender, -(i64)t.amount);
    addToMapElementOrInsertZero(wallets, t.recipiant, (i64)t.amount);
}

void Manager::removeTransactionFromWallets(std::map<str, i64>& wallets, const Transaction& t)
{
    if(t.sender == t.recipiant && t.amount == 2000)
    {
        addToMapElementOrInsertZero(wallets, t.sender, -(i64)t.amount);
        return;
    }

    addToMapElementOrInsertZero(wallets, t.sender, (i64)t.amount);
    addToMapElementOrInsertZero(wallets, t.recipiant, -(i64)t.amount);
}

void Manager::removeTransactionFromCurrentBlock(const Transaction& t)
{
    const auto itTrans = std::find_if(
        std::cbegin(currentBlock.transactions),
        std::cend(currentBlock.transactions),
        [&t](const Transaction& tt)
        {
            return Transaction::hashValue(t) == Transaction::hashValue(tt);
        });

    if(itTrans != std::cend(currentBlock.transactions))
    {
        removeTransactionFromWallets(currentBlockWalletDeltas, t);
        currentBlock.transactions.erase(itTrans);
    }
}