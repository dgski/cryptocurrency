# Control Flow From Posting Transaction

1. Client module sends new transaction to Transactioner
2. Transactioner checks that the message is signed by the client
3. When the next Block has room, Manager will request the transaction for inclusion
4. Manager calculates the hash of the block id, transactions and previous block hash (added the mining reward as a transaction)
5. Manager sends out the block hash to the instances of the Miner module
6. If a Miner module determines the proof of work, it sends it back to the Manager module.
7. The Manager module confirms the proof of work is valid for the most recent block and forwards it to the Network module
8. The Network module propagates the new block to peers.
9. The Manager module starts working on the next Block.

1. If a block of greater id then the one the Manager is working on has been received, and the proof of work is confirmed, the manager abandons the current block, and removes the posted transactions already included in the canonical blockchain.