cmake_minimum_required(VERSION 3.16)

project(RaceMiner)

set(CMAKE_CXX_STANDARD 17)

add_library(miner_flags INTERFACE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Memory")
    target_compile_options(miner_flags INTERFACE -g -O0 -Wall -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(miner_flags INTERFACE -g -O0 -Wall -fsanitize=address)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(miner_flags INTERFACE -O0 -g -Wall)
    target_link_options(miner_flags INTERFACE -O0 -g -Wall)
elseif(CMAKE_BUILD_TYPE STREQUAL "Time")
    target_compile_options(miner_flags INTERFACE -pg)
    target_link_options(miner_flags INTERFACE -pg)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(miner_flags INTERFACE -O3)
    target_link_options(miner_flags INTERFACE -O3)
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(lib)
add_subdirectory(app)
