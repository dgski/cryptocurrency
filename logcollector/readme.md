# The LogCollector Module

Periodically gathers incoming local logs from the other modules. Once saved, a module is given permission to destroy their local copy.

## Incoming Messages
- MSG_MODULE_LOGCOLLECTOR_LOGREADY - Module announces log is ready for archiving
- MSG_MODULE_LOGCOLLECTOR_LOGARCHIVE_CHUNK - A chuck of the archived log

## Outgoing Messages
- MSG_LOGCOLLECTOR_MODULE_LOGREQUEST - A Request to a module asking for the log
- MSG_LOGCOLLECTOR_MODULE_DELETELOCALARCHIVEOK - Signals to module that they can delete local archive

## Connections
- Miners
- Transactioner
- Networker
- Manager

## Configuration Parameters
- logFileName - string value of filename for log output
- logPath - string value of folder to save logs
- connFromModules - ip address and port string