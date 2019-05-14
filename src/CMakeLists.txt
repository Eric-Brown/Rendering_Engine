cmake_minimum_required(VERSION 3.10)

include_directories(includes)
add_executable(DnDIdea main.cpp includes/Shader_Helper.h)
target_link_libraries(DnDIdea PRIVATE glbinding::glbinding glbinding::glbinding-aux glfw glm)
