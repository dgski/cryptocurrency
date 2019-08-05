# Manager
g++-8 -std=c++17 manager/main.cpp shared/Communication.cpp shared/Utils.cpp -o bin/manager

# Miner
g++-8 -std=c++17 miner/main.cpp miner/Miner.cpp shared/Communication.cpp shared/Utils.cpp -o bin/miner

# Transactioner
g++-8 -std=c++17 transactioner/main.cpp shared/Communication.cpp shared/Utils.cpp -o bin/transactioner