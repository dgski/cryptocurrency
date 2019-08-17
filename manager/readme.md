# The Manager Module

The Manager module accepts new transactions from the Transactioner module, constructs a new block and coordinates mining efforts with the Mining module.

## Messages
- Outgoing: MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ - Asks Transactioner module for new Transactions
- Incoming: MSG_A_MANAGER_TRANSACTIONER_TRANSREQ -  Contains New Transactions
- Outgoing: MSG_MANAGER_MINER_NEWBASEHASH - Sends the new Base Hash to the Miner modules
- Incoming: MSG_MINER_MANAGER_PROOFOFWORK - Receives the Proof of Work from the Miner modules

## Threads
- Uses one thread