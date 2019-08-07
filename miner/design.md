# The Miner Module

The Miner module is a simple module with two running threads and one external connection. It's purpose is to find the proof of work (nonce) given a certain base hash.

## Messages
- Outgoing: MSG_MINER_MANAGER_PROOFOFWORK - Contains the proof of work
- Incoming: MSG_MANAGER_MINER_NEWBASEHASH - Contains the new Base Hash to use

## Threads
- Network thread: Receives And Sends Messages
- Miner thread: finds proof of work
- Communication between threads exists using protected variable (AtomicChannel)