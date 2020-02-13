cmake_minimum_required(VERSION 3.10)

set(VULKAN_LIB_LIST "vulkan-1")
set(CMAKE_CXX_STANDARD 17)
add_executable(vulk_main main.cpp
        TriangleApplication.cpp
        TriangleApplication.h
        CommonIncludes.h Vertex.h Vertex.cpp)

target_link_libraries(vulk_main PRIVATE glfw glm ${VULKAN_LIB_LIST} ${ASSIMP_LIBRARIES})
