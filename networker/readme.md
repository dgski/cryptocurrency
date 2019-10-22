# The Networker Module

Responsible for connections to other nodes. Propagates new blocks from the local Manager module to the outside world and forwards new external blocks for consumption by the Manager module.

## Incoming Messages
- MSG_LOGCOLLECTOR_MODULE_LOGREQUEST - Requests log archives
- MSG_MANAGER_NETWORKER_NEWBLOCK - New Block to Propagate from Manager
- MSG_NETWORKER_NETWORKER_REGISTERME - Request from external node asking to be part of block propagation
- MSG_NETWORKER_NETWORKER_NEWBLOCK - Incoming Block from an external Node
- MSG_NETWORKER_NETWORKER_BLOCKREQUEST - A request for a block with the provided id from another Node
MSG_MANAGER_NETWORKER_BLOCK - An externally requested block, requested of local Manager, now in response from said Manager.

## Outgoing Messages
- MSG_MODULE_LOGCOLLECTOR_LOGREADY - Signals to LogCollector that logs are ready to be archived
- MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK - A chunk of the archived log
- MSG_NETWORKER_NETWORKER_NEWBLOCK - Locally mined block for propagation
- MSG_NETWORKER_MANAGER_NEWBLOCK - New incoming foreign block to be propagated to Manager for confirmation
- MSG_NETWORKER_NETWORKER_BLOCKREQUEST - A request for a block with the provided id to another Node
- MSG_NETWORKER_MANAGER_BLOCK - An incoming block, in response to a request
- MSG_NETWORKER_MANAGER_BLOCKREQUEST - forward an external block request to the local Manager
- MSG_NETWORKER_NETWORKER_BLOCK - Requested block response to external Node.

## Connections
- Manager
- LogCollector

## Configuration Parameters
- logFileName - string value of filename for log output
- connToLogCollector - ip address and port string
- connToManager - ip address and port string
- connFromOtherNodes - ip address and port string
- connsToOtherNodes - comma seperated list of ip address and port strings