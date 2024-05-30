cmake_minimum_required(VERSION 3.10)
project(ParaFilter)

# Set the C++ compiler and the flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb ${CMAKE_CXX_FLAGS_USER}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -fuse-linker-plugin -ftree-vectorize ${CMAKE_CXX_FLAGS_USER}")
set(CMAKE_CXX_STANDARD 20)

add_definitions(
    "-march=native"
    "-DTMC_WORK_ITEM=CORO" # one of: CORO, FUNC, FUNCORO, FUNCORO32
)

set(CMAKE_MODULE_PATH
    ${ParaFilter_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

include(cmake/CPM.cmake)

# Include directories
# include_directories(
# "/usr/lib/x86_64-linux-gnu/openmpi/include"
# "/usr/lib/x86_64-linux-gnu/openmpi/include/openmpi"
# )
include_directories(${CMAKE_SOURCE_DIR}/src/include ${CMAKE_SOURCE_DIR})

CPMAddPackage(
    NAME TooManyCooks
    GIT_REPOSITORY https://github.com/tzcnt/TooManyCooks.git
    GIT_TAG 82a0e7ace8857af9fa8bdc6635c7b9a98371a2ba
    DOWNLOAD_ONLY)

include_directories(
    ${TooManyCooks_SOURCE_DIR}/include
)

# HWLOC support
option(TMC_USE_HWLOC "libhwloc" ON)

if(TMC_USE_HWLOC)
    message(STATUS "TMC_USE_HWLOC: running find_package(libhwloc)...")
    find_package(libhwloc)

    if(NOT LIBHWLOC_FOUND)
        message(FATAL_ERROR "TMC_USE_HWLOC: ERROR package libhwloc not found")
    else()
        message(STATUS "TMC_USE_HWLOC: using ${LIBHWLOC_LIBRARY}")
        add_compile_definitions(TMC_USE_HWLOC)
        link_libraries(${LIBHWLOC_LIBRARY})
    endif()
endif()

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

add_custom_command(TARGET bin-bench POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_LIST_DIR}/build
)