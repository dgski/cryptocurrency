add_executable(manager
    Manager.cpp
    main.cpp
    ../shared/Module.cpp
    ../shared/Communication.cpp
    ../shared/Utils.cpp
    ../shared/Crypto.cpp
)

target_link_libraries(manager -lcrypto)
target_link_libraries(manager -lstdc++fs)

target_compile_features(manager PUBLIC cxx_std_17)