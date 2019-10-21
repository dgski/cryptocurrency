add_executable(networker
    Networker.cpp
    main.cpp
)

target_link_libraries(networker shared)
target_compile_features(networker PUBLIC cxx_std_17)