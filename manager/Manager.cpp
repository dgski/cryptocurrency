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
            processNetworkerChainRequest(msg);
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

    if(alreadyValidatingForeignChain)
    {
        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(msg.reqId));

        return;
    }

    alreadyValidatingForeignChain = true;

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

    std::list<Block> potentialChain{ std::move(incoming.block) };
    tryAbsorbChain(msg.reqId, std::move(potentialChain));
}

void Manager::tryAbsorbChain(u32 reqId, std::list<Block> potentialChain)
{
    logger.logInfo({
        {"potentialChain.size()", (u64)potentialChain.size()}
    });

    if(!potentialChain.front().isValid())
    {
        logger.logInfo({
            {"event", "Block is not valid; not requesting further blocks"}
        });

        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(reqId));

        return;
    }

    if(potentialChain.front().id == 0)
    {
        MSG_MANAGER_NETWORKER_BLOCKREQUEST outgoing;
        outgoing.voidRequest = true;
        connFromNetworker.sendMessage(outgoing.msg(reqId));

        finalizeAbsorbChain(std::move(potentialChain));
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
    // Check that in the meantime we did not accumulate a bigger chain
    // TODO

    logger.logInfo("Foreign chain valid. Absorbing.");

    // Splice the current chain into that range
    // TODO

    // Remove effects of removed blocks, Add effects of added blocks
    // TODO

    // Start working on new Block
    currentBlock.id = chain.back().id + 1;
    currentBlock.hashOfLastBlock = chain.back().calculateFullHash();
    currentBlockWalletDeltas.clear();

    std::remove_if(
        std::begin(currentBlock.transactions),
        std::end(currentBlock.transactions),
        [this](Transaction& t)
        {
            return true; // TODO: Remove if transactions are already in Chain
        });

    mintCurrency();
    sendBaseHashToMiners();
    alreadyValidatingForeignChain = false;
}

void Manager::processNetworkerChainRequest(const Message& msg)
{
    MSG_NETWORKER_MANAGER_BLOCKREQUEST incoming{ msg };

    MSG_MANAGER_NETWORKER_BLOCK outgoing;
    outgoing.block = *std::find_if(
        std::begin(chain),
        std::end(chain),
        [&incoming](const Block& block)
        {
            return block.id == incoming.blockId;
        }); // Slow operation for now
    
    connFromNetworker.sendMessage(outgoing.msg(msg.reqId));
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