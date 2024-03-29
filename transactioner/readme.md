# The Transactioner Module

Accepts incoming transactions from clients. Holds all valid transactions in a waiting memory pool. A connected Manager module requests these transactions when space is available on the next block.

## Incoming Messages
- MSG_LOGCOLLECTOR_MODULE_LOGREQUEST - Requests log archives
- MSG_CLIENT_TRANSACTIONER_NEWTRANS - Receives a new transaction from a Client
- MSG_MANAGER_TRANSACTIONER_TRANSREQ - Request from Manager for new Transactions
- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET_REPLY - Reply about wallet inquiry

## Outgoing Messages
- MSG_MODULE_LOGCOLLECTOR_LOGREADY - Signals to LogCollector that logs are ready to be archived
- MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK - A chunk of the archived log
- MSG_MANAGER_TRANSACTIONER_TRANSREQ_REPLY -  Contains New Transactions
- MSG_TRANSACTIONER_MANAGER_FUNDSINWALLET - Inquires about the funds available in this wallet

## Connections
- Clients
- Manager
- LogCollector

## Configuration Parameters
- logFileName - string value of filename for log output
- logPath - string value of folder to save logs
- connToManager - ip address and port string
- connFromClients - ip address and port string