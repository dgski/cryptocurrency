# Fake Client
g++-8 -std=c++17 fake_client/main.cpp shared/Communication.cpp shared/Utils.cpp shared/Crypto.cpp -o bin/fake_client -lcrypto -lstdc++fs