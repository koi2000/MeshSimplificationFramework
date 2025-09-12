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

function(collect_source_files DIRECTORY_NAME)
    file(GLOB_RECURSE ${DIRECTORY_NAME}_SOURCES
        "src/${DIRECTORY_NAME}/*.h"
        "src/${DIRECTORY_NAME}/*.cpp"
    )
    set(${DIRECTORY_NAME}_SOURCES ${${DIRECTORY_NAME}_SOURCES} PARENT_SCOPE)
endfunction()

add_subdirectory(deps/MCGAL)

# 自动收集所有子目录
collect_source_files(common)
collect_source_files(compress)
collect_source_files(decompress)
collect_source_files(elimination)
collect_source_files(handler)
collect_source_files(header)
collect_source_files(operator)
collect_source_files(options)
collect_source_files(reconstruct)
collect_source_files(segmentation)
collect_source_files(selection)

# 收集框架核心文件
file(GLOB_RECURSE FRAMEWORK
    "src/PMSF.h"
    "src/Options.h"
    "src/PMSF.cpp"
)

# 合并所有源文件
set(ALL_SOURCES
    ${FRAMEWORK}
    ${common_SOURCES}
    ${compress_SOURCES}
    ${decompress_SOURCES}
    ${elimination_SOURCES}
    ${handler_SOURCES}
    ${header_SOURCES}
    ${operator_SOURCES}
    ${options_SOURCES}
    ${reconstruct_SOURCES}
    ${segmentation_SOURCES}
    ${selection_SOURCES}
)

find_package(ZLIB REQUIRED)

add_executable(DefaultCompress tools/DefaultCompress.cpp ${ALL_SOURCES})
target_include_directories(DefaultCompress PUBLIC "src" )
target_link_libraries(DefaultCompress MCGAL_static ZLIB::ZLIB)

add_executable(DefaultDecompress tools/DefaultDecompress.cpp ${ALL_SOURCES})
target_include_directories(DefaultDecompress PUBLIC "src" )
target_link_libraries(DefaultDecompress MCGAL_static ZLIB::ZLIB)


get_property(targetIncludes DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "Target Include Directories: ${targetIncludes}")
