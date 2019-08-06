# Manager
g++-8 -std=c++17 manager/main.cpp manager/Manager.cpp shared/Communication.cpp shared/Utils.cpp -o bin/manager

# Miner
g++-8 -std=c++17 miner/main.cpp miner/Miner.cpp shared/Communication.cpp shared/Utils.cpp -o bin/miner

# Transactioner
g++-8 -std=c++17 transactioner/main.cpp transactioner/Transactioner.cpp shared/Communication.cpp shared/Utils.cpp -o bin/transactioner

# Fake Client
g++-8 -std=c++17 fake_client/main.cpp shared/Communication.cpp shared/Utils.cpp -o bin/fake_client