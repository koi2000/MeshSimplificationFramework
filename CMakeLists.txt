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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wno-unused-result -lpthread -ljemalloc")

set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -allow-unsupported-compiler -rdc=true -arch=compute_89 -code=sm_89 -Xcompiler -std=c++17 -g -G -O0")

message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

include_directories(
    /usr/include
    # /usr/local/include
    /usr/local/include/OpenMesh/include/
    src/include
    deps/MCGAL/Core/include

    /usr/local/cgal/include
    /usr/include/jemalloc
    ${CUDA_INCLUDE_DIRS}
)

set(MINICGAL_SOURCE_DIR "deps/MCGAL/Core")

file(GLOB_RECURSE MINI_CGAL_FILES
    "${MINICGAL_SOURCE_DIR}/include/*.h"
    "${MINICGAL_SOURCE_DIR}/include/openmesh/*.hh"
    "${MINICGAL_SOURCE_DIR}/include/openmesh/System/*.hh"
    "${MINICGAL_SOURCE_DIR}/include/*.hpp"
    "${MINICGAL_SOURCE_DIR}/*.cpp"
    "${MINICGAL_SOURCE_DIR}/*.cc"
    "${MINICGAL_SOURCE_DIR}/*.hpp"
)

file(GLOB_RECURSE SIMPLE_ENCODER_FILES
    "src/include/SimpleEncoder.h"
    "src/SimpleEncoder.cpp"
    "src/SimpleDump.cpp"
    "src/SimpleStrategy.cpp"
)

file(GLOB_RECURSE MULTI_SIMPLE_ENCODER_FILES
    "src/common/Graph.h"
    "src/include/MultiSimpleEncoder.h"
    "src/include/PartialSplitter.h"
    "src/MultiSimpleEncoder.cpp"
    "src/MultiSimpleDump.cpp"
    "src/MultiSimpleStrategy.cpp"
    "src/PartialSplitter.cpp"
)

file(GLOB_RECURSE SIMPLE_DECODER_FILES
    "src/include/SimpleDecoder.h"
    "src/SimpleDecoder.cpp"
)

find_package(OpenMP REQUIRED)
find_package(CUDA REQUIRED)

find_library(JEMALLOC_LIB jemalloc REQUIRED)

set(LIBS_DEBUG libOpenMeshCore.a libOpenMeshCore.so libOpenMeshTools.a libOpenMeshTools.so)
set(LIBS_RELEASE libOpenMeshCore.a libOpenMeshCore.so libOpenMeshTools.a libOpenMeshTools.so)

add_executable(SimpleEncoderTest tools/SimpleEncoderTest.cpp ${SIMPLE_ENCODER_FILES} ${MINI_CGAL_FILES})
target_include_directories(SimpleEncoderTest PUBLIC "deps/MCGAL/Core/include" "src/include")
target_link_libraries(SimpleEncoderTest OpenMP::OpenMP_CXX  ${JEMALLOC_LIB} ${LIBS_DEBUG})

add_executable(MultiSimpleEncoderTest tools/MultiSimpleEncoderTest.cpp ${MULTI_SIMPLE_ENCODER_FILES} ${MINI_CGAL_FILES})
target_include_directories(MultiSimpleEncoderTest PUBLIC "deps/MCGAL/Core/include" "src/include")
target_link_libraries(MultiSimpleEncoderTest OpenMP::OpenMP_CXX  ${JEMALLOC_LIB} ${LIBS_DEBUG})

add_executable(SimpleDecoderTest tools/SimpleDecoderTest.cpp ${SIMPLE_DECODER_FILES} ${MINI_CGAL_FILES})
target_include_directories(SimpleDecoderTest PUBLIC "deps/MCGAL/Core/include" "src/include")
target_link_libraries(SimpleDecoderTest OpenMP::OpenMP_CXX  ${JEMALLOC_LIB} ${LIBS_DEBUG})


get_property(targetIncludes DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "Target Include Directories: ${targetIncludes}")
