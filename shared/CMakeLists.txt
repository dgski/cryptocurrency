add_library(shared
    Module.cpp
    Communication.cpp
    Utils.cpp
    Crypto.cpp
)
target_compile_features(shared PUBLIC cxx_std_17)
target_link_libraries(shared -lcrypto -lstdc++fs)