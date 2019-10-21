add_executable(transactioner
    Transactioner.cpp
    main.cpp
)

target_link_libraries(transactioner shared)
target_compile_features(transactioner PUBLIC cxx_std_17)