# Manager
g++-8 -std=c++17 manager/main.cpp manager/Manager.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Transaction.cpp -o bin/manager -lcrypto

# Miner
g++-8 -std=c++17 miner/main.cpp miner/Miner.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Transaction.cpp -o bin/miner -lcrypto

# Transactioner
g++-8 -std=c++17 transactioner/main.cpp transactioner/Transactioner.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Transaction.cpp -o bin/transactioner -lcrypto

# Networker
g++-8 -std=c++17 networker/main.cpp networker/Networker.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Transaction.cpp -o bin/networker -lcrypto

# Fake Client
g++-8 -std=c++17 fake_client/main.cpp shared/Communication.cpp shared/Utils.cpp shared/Transaction.cpp -o bin/fake_client -lcrypto