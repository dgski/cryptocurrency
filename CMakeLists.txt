cmake_minimum_required(VERSION 3.13)
project(cryptocurrency)

set(EXECUTABLE_OUTPUT_PATH "../../bin")

##! Add subprojects here
add_subdirectory(shared)
add_subdirectory(logcollector)
add_subdirectory(manager)
add_subdirectory(miner)
add_subdirectory(networker)
add_subdirectory(transactioner)