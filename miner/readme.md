# The Miner Module

Continually tries to calculate the proof of work using a base value representing the in-progress block (Received from the Manager module). When proof of work is found, value is sent to the Manager module, and mining is suspended until Manager begins constructing a new Block.

## Outgoing Messages
- MSG_LOGCOLLECTOR_MODULE_LOGREQUEST - Requests log archives
- MSG_MINER_MANAGER_PROOFOFWORK - Contains the proof of work

## Incoming Messages
- MSG_MODULE_LOGCOLLECTOR_LOGREADY - Signals to LogCollector that logs are ready to be archived
- MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK - A chunk of the archived log
- MSG_MANAGER_MINER_NEWBASEHASH - Contains the new Base Hash to use

## Connections
- Manager
- LogCollector

## Configuration Parameters
- logFileName - string value of filename for log output
- connToLogCollector - ip address and port string
- connToManager - ip address and port string
- incrementSize - integer value of nonce incrementation during mining