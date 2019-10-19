add_executable(transactioner
    Transactioner.cpp
    main.cpp
    ../shared/Module.cpp
    ../shared/Communication.cpp
    ../shared/Utils.cpp
    ../shared/Crypto.cpp
)

target_link_libraries(transactioner -lcrypto)
target_link_libraries(transactioner -lstdc++fs)

target_compile_features(transactioner PUBLIC cxx_std_17)