# The Networker Module

Responsible for connections to other nodes. Propagates new blocks from the local Manager module to the outside world and forwards new external blocks for consumption by the Manager module.

## Incoming Messages
- MSG_MANAGER_NETWORKER_NEWBLOCK - New Block to Propagate from Manager
- MSG_NETWORKER_NETWORKER_REGISTERME - Request from external node asking to be part of block propagation
- MSG_NETWORKER_NETWORKER_NEWBLOCK - Incoming Block from an external Node
- MSG_MANAGER_NETWORKER_CHAINREQUEST - A chain request to an external node from the Manager module
- MSG_NETWORKER_NETWORKER_CHAINREQUEST - A chain request from an external node to the Manager module

## Outgoing Messages
- MSG_NETWORKER_NETWORKER_NEWBLOCK - New Block for Other Nodes
- MSG_NETWORKER_MANAGER_NEWBLOCK - New Block from other nodes for manager
- MSG_NETWORKER_NETWORKER_CHAINREQUEST - Request for a chain for another node
- MSG_NETWORKER_MANAGER_CHAIN - A chain for the Manager module
- MSG_NETWORKER_MANAGER_CHAINREQUEST - A chain request from an external node to the Manager module
- MSG_NETWORKER_NETWORKER_CHAIN - A chain from an external node

## Configuration Parameters
- logFileName - string value of filename for log output
- connToLogCollector - ip address and port string
- connToManager - ip address and port string
- connFromOtherNodes - ip address and port string
- connsToOtherNodes - comma seperated list of ip address and port strings