# The Manager Module

The Manager module accepts new transactions from the transaction pool, constructs a new block, maintains the current blockchain, and coordinates mining efforts.

## Messages
- Incoming: New Transaction From Transaction Pool
- Incoming/Outgoing: Miners (Send New Base Hash/ Receive Proof)
- Incoming/Outgoing: Networker (Receive New Block/ Send New Block)

## Threads
- Uses one thread