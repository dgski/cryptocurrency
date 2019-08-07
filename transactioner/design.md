# The Transactioner Module

The Transactioner module receives and verifies the signatures attached to new transactions from Clients, and holds them in a pool until the Manager module has space on the next block and requests new transactions;

## Messages
- Incoming: MSG_CLIENT_TRANSACTIONER_NEWTRANS - Receives a new transaction from a Client
- Incoming: MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ - Request from Manager for new Transactions
- Outgoing: MSG_A_MANAGER_TRANSACTIONER_TRANSREQ -  Contains New Transactions

## Threads
- One Thread