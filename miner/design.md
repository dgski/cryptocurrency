# The Miner Module

The Miner module is a simple module with two running threads and one external connection. It's purpose is to find the proof of work (nonce) given a certain base hash.

## Connection To Manager Module
- Outgoing: Request Base Hash, Notify On Find of Nonce
- Incoming: New Base Hash

## Threads
- Network thread: Receives And Sends Messages
- Miner thread: finds proof of work
- Communication between threads exists using protected variable