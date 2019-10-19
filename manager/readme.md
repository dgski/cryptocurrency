# The Manager Module

Assembles the next block by requesting waiting transactions from the Transactioner module, and coordinating mining efforts of the Miner modules. Once proof of work is received from a Miner, it sends out the Block to the Networker module; to be propagated to other nodes. Also validates blocks from other nodes, absorbing them if they belong to a longer chain.

## Incoming Messages
- MSG_A_MANAGER_TRANSACTIONER_TRANSREQ -  Contains New Transactions
- MSG_MINER_MANAGER_PROOFOFWORK - Receives the Proof of Work from the Miner modules
- MSG_MINER_MANAGER_HASHREQUEST - Receives a request for the base hash
- MSG_NETWORKER_MANAGER_NEWBLOCK - Receives a new block from external node
- MSG_NETWORKER_MANAGER_CHAINREQUEST - Receives a request for the full chain from an external node
- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET - Request for the number of currency units in a given  wallet

## Outgoing Messages
- MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ - Asks Transactioner module for new Transactions
- MSG_MANAGER_MINER_NEWBASEHASH - Sends the new Base Hash to the Miner modules
- MSG_MANAGER_NETWORKER_CHAIN - Sends the current chain to the networker module

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