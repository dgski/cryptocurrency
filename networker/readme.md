# The Networker Module

The Networker module accepts new Blocks from the Manager module to propagate to other Nodes, as well as Blocks from other nodes to the Manager Module.

## Messages
- Incoming: MSG_MANAGER_NETWORKER_NEWBLOCK - New Block to Propagate from Manager
- Incoming: MSG_NETWORKER_NETWORKER_NEWBLOCK - New Block for Manager
- Outgoing: MSG_NETWORKER_NETWORKER_NEWBLOCK - New Block for Other Nodes
- TODO Outgoing -> forward new block

## Threads
- Uses one thread