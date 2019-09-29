# Manager
g++-8 -std=c++17 manager/main.cpp manager/Manager.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Crypto.cpp -o bin/manager -lcrypto -lstdc++fs

# Miner
g++-8 -std=c++17 miner/main.cpp miner/Miner.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Crypto.cpp -o bin/miner -lcrypto -lstdc++fs

# Transactioner
g++-8 -std=c++17 transactioner/main.cpp transactioner/Transactioner.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Crypto.cpp -o bin/transactioner -lcrypto -lstdc++fs

# Networker
g++-8 -std=c++17 networker/main.cpp networker/Networker.cpp shared/Module.cpp shared/Communication.cpp shared/Utils.cpp shared/Crypto.cpp -o bin/networker -lcrypto -lstdc++fs

# Fake Client
g++-8 -std=c++17 fake_client/main.cpp shared/Communication.cpp shared/Utils.cpp shared/Crypto.cpp -o bin/fake_client -lcrypto -lstdc++fs