add_executable(networker
    Networker.cpp
    main.cpp
    ../shared/Module.cpp
    ../shared/Communication.cpp
    ../shared/Utils.cpp
    ../shared/Crypto.cpp
)

target_link_libraries(networker -lcrypto)
target_link_libraries(networker -lstdc++fs)

target_compile_features(networker PUBLIC cxx_std_17)