cmake_minimum_required(VERSION 3.10)
project(ParaFilter)

# Set the C++ compiler and the flags
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -fuse-linker-plugin -ftree-vectorize")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src/include ${CMAKE_SOURCE_DIR})

# Find all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp" "src/*.cpp")
file(GLOB_RECURSE BENCH_SOURCES "benchmark/*.cpp" "src/*.cpp")

# Create the main executable
add_executable(bin-debug ${SOURCES})
add_executable(bin-release ${SOURCES})

list(FILTER TEST_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
list(FILTER BENCH_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# MPI version
find_package(MPI REQUIRED)
add_executable(bin-debug-mpi ${SOURCES})
target_compile_definitions(bin-debug-mpi PRIVATE USE_MPI=1)
target_link_libraries(bin-debug-mpi MPI::MPI_CXX)

# Testing executable
list(FILTER SOURCES EXCLUDE REGEX ".*main.cpp$")
add_executable(bin-test ${TEST_SOURCES})
target_link_libraries(bin-test gtest gtest_main pthread)

# Benchmark executable
add_executable(bin-bench ${BENCH_SOURCES})
target_link_libraries(bin-bench benchmark pthread)

# Add custom targets for clean and test
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/clean.cmake
)

enable_testing()
add_test(NAME TestAll COMMAND bin-test)
add_custom_target(test-bin
    COMMAND ${CMAKE_TEST_COMMAND} --verbose
)

add_custom_target(bench
    COMMAND ./bin-bench
)
