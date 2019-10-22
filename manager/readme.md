# The Manager Module

Assembles the next block by requesting waiting transactions from the Transactioner module, and coordinating mining efforts of the Miner modules. Once proof of work is received from a Miner, it sends out the Block to the Networker module; to be propagated to other nodes. Also validates blocks from other nodes, absorbing them if they belong to a longer chain.

## Outgoing Messages
- MSG_LOGCOLLECTOR_MODULE_LOGREQUEST - Requests log archives
- MSG_MANAGER_TRANSACTIONER_TRANSREQ - Asks Transactioner module for new Transactions
- MSG_MANAGER_MINER_NEWBASEHASH - Sends the new Base Hash to the Miner modules
- MSG_MANAGER_NETWORKER_NEWBLOCK - Mined block being sent to Networker for propagation
- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY - Response containing the currency within request wallet
- MSG_MANAGER_NETWORKER_BLOCKREQUEST - Block request to external node, routed through Networker (can be flagged void to signal end of block requests)
- MSG_MANAGER_NETWORKER_BLOCK - Response to external request

## Incoming Messages
- MSG_MODULE_LOGCOLLECTOR_LOGREADY - Signals to LogCollector that logs are ready to be archived
- MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK - A chunk of the archived log
- MSG_MANAGER_TRANSACTIONER_TRANSREQ_REPLY -  Contains New Transactions
- MSG_MINER_MANAGER_PROOFOFWORK - Receives the Proof of Work from the Miner modules
- MSG_MINER_MANAGER_HASHREQUEST - Receives a request for the base hash
- MSG_NETWORKER_MANAGER_NEWBLOCK - New block from external node
- MSG_NETWORKER_MANAGER_BLOCKREQUEST - A block request from a external node
- MSG_NETWORKER_MANAGER_BLOCK - A block, in response to a request from an external node


- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET - Request for the number of currency units in a given  wallet


## Connections
- Miners
- Transactioner
- Networker
- LogCollector

## Configuration Parameters
- logFileName - string value of filename for log output
- connToLogCollector - ip address and port string
- connFromMiners - ip address and port string
- connFromTransactioner - ip address and port string
- connFromNetworker - ip address and port string
- privateKeyFile - string value of filename with private wallet key that will receive funds
- publicKeyFile - string value of filename with public wallet key that will receive funds