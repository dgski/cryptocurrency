add_executable(manager
    Manager.cpp
    main.cpp
)

target_link_libraries(manager shared)
target_compile_features(manager PUBLIC cxx_std_17)