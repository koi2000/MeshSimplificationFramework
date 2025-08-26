cmake_minimum_required(VERSION 3.18)

# export CC=/home/koi/install/gcc-12/bin/gcc
# export CXX=/home/koi/install/gcc-12/bin/g++

# ./CudaPartialDecoderTest > output.txt 2>&1
# for local
set(CMAKE_C_COMPILER "/usr/local/gcc-12/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/local/gcc-12/bin/g++")
# set(CMAKE_CUDA_COMPILER "/usr/local/cuda-12.3/bin/nvcc")
# for remote
# set(CMAKE_CUDA_COMPILER "/usr/local/cuda-12.2/bin/nvcc")


message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
project(LocalDecodeManager LANGUAGES CUDA CXX C)
set(CMAKE_CUDA_ARCHITECTURES 89) 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wno-unused-result -lpthread -ljemalloc")

set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -allow-unsupported-compiler -rdc=true -arch=compute_89 -code=sm_89 -Xcompiler -std=c++17 -g -G -O0")

message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

include_directories(
    /usr/include
    /usr/local/include/OpenMesh/include/
    src/include/
    src/
    /usr/include/jemalloc
    ${CUDA_INCLUDE_DIRS}
)

add_subdirectory(deps/MCGAL)

file(GLOB_RECURSE HANDLER
    "src/handler/*.h"
    "src/handler/*.cpp"
)

file(GLOB_RECURSE OPERATOR
    "src/operator/*.h"
    "src/operator/*.cpp"
)


file(GLOB_RECURSE FRAMEWORK
    "src/PMSF.h"
    "src/PMSF.cpp"
)

add_executable(PMSFTest tools/PMSF_Test.cpp ${FRAMEWORK} ${HANDLER} ${OPERATOR})
target_include_directories(PMSFTest PUBLIC "src")
target_link_libraries(PMSFTest MCGAL_static)


get_property(targetIncludes DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "Target Include Directories: ${targetIncludes}")
