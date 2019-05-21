cmake_minimum_required(VERSION 3.10)

set(VULKAN_LIB_LIST "vulkan-1")
add_executable(vulk_main main.cpp)

target_link_libraries(vulk_main PRIVATE glfw glm ${VULKAN_LIB_LIST})
