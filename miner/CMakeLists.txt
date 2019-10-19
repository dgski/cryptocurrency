add_executable(miner
    Miner.cpp
    main.cpp
    ../shared/Module.cpp
    ../shared/Communication.cpp
    ../shared/Utils.cpp
    ../shared/Crypto.cpp
)

target_link_libraries(miner -lcrypto)
target_link_libraries(miner -lstdc++fs)

target_compile_features(miner PUBLIC cxx_std_17)