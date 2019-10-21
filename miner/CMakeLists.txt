add_executable(miner
    Miner.cpp
    main.cpp
)

target_link_libraries(miner shared)
target_compile_features(miner PUBLIC cxx_std_17)