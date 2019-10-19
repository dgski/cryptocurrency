add_executable(logcollector
    LogCollector.cpp
    main.cpp
    ../shared/Module.cpp
    ../shared/Communication.cpp
    ../shared/Utils.cpp
    ../shared/Crypto.cpp
)

target_link_libraries(logcollector -lcrypto)
target_link_libraries(logcollector -lstdc++fs)

target_compile_features(logcollector PUBLIC cxx_std_17)