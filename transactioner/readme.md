# The Transactioner Module

The Transactioner module receives and verifies the signatures attached to new transactions from Clients, and holds them in a pool until the Manager module has space on the next block and requests new transactions;

## Incoming Messages
- MSG_CLIENT_TRANSACTIONER_NEWTRANS - Receives a new transaction from a Client
- MSG_Q_MANAGER_TRANSACTIONER_TRANSREQ - Request from Manager for new Transactions
- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY - Reply about wallet inquiry

## Outgoing Messages
- MSG_A_MANAGER_TRANSACTIONER_TRANSREQ -  Contains New Transactions
- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET - Inquires about the funds available in this wallet

## Configuration Parameters
- connToManager - ip address and port string
- connFromClients - ip address and port string