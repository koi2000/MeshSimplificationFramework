cmake_minimum_required(VERSION 3.10)
project(MCGAL)

# 收集源文件
file(GLOB MCGAL_SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/openmesh/*.cc"

)

# 设置头文件路径
set(MCGAL_INCLUDE_DIR 
    "${CMAKE_CURRENT_SOURCE_DIR}/include",
    "${CMAKE_CURRENT_SOURCE_DIR}/include/openmesh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/openmesh/System"
    "/usr/local/include/OpenMesh/include/"
)

find_package(OpenMP REQUIRED)

set(LIBS_DEBUG libOpenMeshCore.a libOpenMeshCore.so libOpenMeshTools.a libOpenMeshTools.so)
set(LIBS_RELEASE libOpenMeshCore.a libOpenMeshCore.so libOpenMeshTools.a libOpenMeshTools.so)

# 静态库
add_library(MCGAL_static STATIC ${MCGAL_SRC})
target_include_directories(MCGAL_static PUBLIC ${MCGAL_INCLUDE_DIR})
target_link_libraries(MCGAL_static ${LIBS_DEBUG})


# 动态库
add_library(MCGAL_shared SHARED ${MCGAL_SRC})
target_include_directories(MCGAL_shared PUBLIC ${MCGAL_INCLUDE_DIR})
target_link_libraries(MCGAL_shared ${LIBS_DEBUG})


# 源码方式就是直接用上面 static/shared 的target