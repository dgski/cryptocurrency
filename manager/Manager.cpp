#include "Manager.h"

Manager::Manager(const char* iniFileName)
{
    const std::map<str,str> params = getInitParameters(iniFileName);

    log("Manager module starting");

    connFromMiners.init(strToIp(params.at("connFromMiners")));
    registerServerConnection(&connFromMiners);

    connFromTransactioner.init(strToIp(params.at("connFromTransactioner")));
    registerServerConnection(&connFromTransactioner);

    connFromNetworker.init(strToIp(params.at("connFromNetworker")));
    registerServerConnection(&connFromNetworker);

    registerRepeatedTask([this]()
    {
        if(currentBlock.transactions.size() < 200)
        {
            askTransactionerForNewTransactions();
        }
        std::this_thread::sleep_for (std::chrono::seconds(1));
    });

    currentBaseHash = 12393939334343; // FAKE
}

void Manager::processMessage(const Message& msg)
{
    log("processMessage");

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
    MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ contents;
    contents.numOfTransactionsRequested = 200 - currentBlock.transactions.size();

    Message msg;
    contents.compose(msg);

    connFromTransactioner.sendMessage(msg, [this](Message& reply)
    {
        processTransactionRequestReply(reply);
    });
}

void Manager::processTransactionRequestReply(Message& msg)
{
    MSG_A_MANAGER_TRANSACTIONER_TRANSREQ contents{ msg };

    log(
        "processTransactionRequestReply recieved % transactions",
        contents.transactions.size()
    );
    
    std::move(
        begin(contents.transactions),
        end(contents.transactions),
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
    Message hashMsg;
    contents.compose(hashMsg);
    connFromMiners.sendMessage(hashMsg);
}

void Manager::processIncomingProofOfWork(const Message& msg)
{
    MSG_MINER_MANAGER_PROOFOFWORK contents{ msg };
    log("MSG_MINER_MANAGER_PROOFOFWORK proof=%", contents.proofOfWork);

    if(validProof(contents.proofOfWork, currentBaseHash))
    {
        log("Proof is valid, Sending to Networker for Propagation.");
        currentBlock.proofOfWork = contents.proofOfWork;

        // saveToChain
        chain.push_back(currentBlock);

        MSG_MANAGER_NETWORKER_NEWBLOCK blockContents;
        blockContents.block = currentBlock;
        Message blockMsg;
        blockContents.compose(blockMsg);
        connFromNetworker.sendMessage(blockMsg);
        
        log("Starting work on next block.");
        
        Block newBlock;
        newBlock.id = currentBlock.id + 1;
        newBlock.hashOfLastBlock = currentBlock.calculateFullHash();
        
        currentBlock = newBlock;
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

    MSG_MANAGER_NETWORKER_CHAINREQUEST chainRequest;
    chainRequest.maxId = contents.block.id;
    chainRequest.connId = contents.connId;

    Message requestMsg;
    chainRequest.compose(requestMsg);

    connFromNetworker.sendMessage(requestMsg, [this](const Message& msg)
    {
        processPotentialWinningBlock_ChainReply(msg);
    });
}

void Manager::processPotentialWinningBlock_ChainReply(const Message& msg)
{

    /*
    currentBlock.id = winningBlock.id + 1;
    currentBlock.hashOfLastBlock = winningBlock.calculateFullHash();

    std::remove_if(
        currentBlock.transactions.begin(),
        currentBlock.transactions.end(),
        [&winningBlock, this](const Transaction& currTrans)
        {
            auto& accepted = winningBlock.transactions;
            auto it = std::find_if(
                accepted.begin(),
                accepted.end(),
                [currTrans](const Transaction& acceptedTrans)
                {
                    return currTrans.signature == acceptedTrans.signature;
                });

            return it != currentBlock.transactions.end();
        }
    );

    chain.emplace_back(std::move(winningBlock));
    currentBaseHash = currentBlock.calculateBaseHash();
    sendBaseHashToMiners();
    */
}

void Manager::processPotentialWinningBlock_Finalize()
{

}

void Manager::processNetworkerChainRequest(const Message& msg)
{
    MSG_NETWORKER_MANAGER_CHAINREQUEST contents{ msg };

    MSG_MANAGER_NETWORKER_CHAIN replyContents;

    std::copy_if(
        chain.begin(),
        chain.end(),
        std::back_inserter(replyContents.chain),
        [&contents](const Block& block)
        {
            return block.id >= contents.maxId;
        }
    );

    Message replyMessage;
    replyMessage.reqId = msg.reqId;
    replyContents.compose(replyMessage);
    
    connFromNetworker.sendMessage(replyMessage);
}