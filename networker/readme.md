# The Networker Module

The Networker module accepts new Blocks from the Manager module to propagate to other Nodes, as well as Blocks from other nodes to the Manager Module.

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