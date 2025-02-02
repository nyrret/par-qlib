cmake_minimum_required(VERSION 3.14)

# set the project name
project(ParallelQuantLib VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(external/googletest)
add_subdirectory(src)
add_subdirectory(src/test)

include_directories(external/googletest/include)
include_directories(src)
include_directories(src/test)

configure_file(src/main.h.in src/main.h)

# parlaylib
find_package(Threads REQUIRED)
find_package(Parlay CONFIG REQUIRED)

# add the executables
add_executable(ParallelQuantLib src/main.cpp)
add_executable(Test src/test/test.cpp)
add_executable(StencilTest src/test/test-stencil.cpp)
add_executable(PerfTest src/perf-test.cpp)

# link libraries
target_link_libraries(ParallelQuantLib PRIVATE Parlay::parlay)

target_link_libraries(Test PRIVATE gtest_main) # link gtest to this executable
target_link_libraries(Test PRIVATE OriginalBinomial) 

target_link_libraries(StencilTest PRIVATE gtest_main) # link gtest to this executable

target_link_libraries(PerfTest PRIVATE gtest_main) # link gtest to this executable
target_link_libraries(PerfTest PRIVATE OriginalBinomial) 

# add the binary tree to the search path for include files
target_include_directories(ParallelQuantLib PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           "${PROJECT_SOURCE_DIR}/src"
                           )
