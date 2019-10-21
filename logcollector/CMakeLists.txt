add_executable(logcollector
    LogCollector.cpp
    main.cpp
)

target_link_libraries(logcollector shared)
target_compile_features(logcollector PUBLIC cxx_std_17)