cmake_minimum_required(VERSION 3.10)
project(ParaFilter)

# Set C++ compiler
set(CMAKE_CXX_COMPILER g++)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler options
add_compile_options(-Wall -Wextra)
add_compile_definitions($<$<CONFIG:DEBUG>:DEBUG>)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src/include)

# Source files and test files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE SOURCES_NOMAIN "src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# Exclude the main.cpp file for the test build
list(FILTER SOURCES_NOMAIN EXCLUDE REGEX ".*main\\.cpp$")

# Create the debug and release binaries
add_executable(bin-debug ${SOURCES})
target_compile_options(bin-debug PRIVATE -O0 -g -ggdb -MP -MD)

add_executable(bin-release ${SOURCES})
target_compile_options(bin-release PRIVATE -O3 -MP -MD)

# Create the test binary
add_executable(bin-test ${SOURCES_NOMAIN} ${TEST_SOURCES})
target_link_libraries(bin-test PRIVATE gtest gtest_main pthread)
target_compile_options(bin-test PRIVATE -O0 -g -ggdb -MP -MD)

# Custom target for clean command
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR})

# Test command
enable_testing()
add_test(NAME GoogleTest COMMAND bin-test)
