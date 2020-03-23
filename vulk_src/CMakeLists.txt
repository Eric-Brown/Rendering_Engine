cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(Vulk_Main)

add_executable(${PROJECT_NAME} main.cpp
        TriangleApplication.cpp
        TriangleApplication.h
        Vertex.h Vertex.cpp
        ExternalIncludes.h
        Model.cpp Model.h)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm Vulkan::Vulkan Assimp::assimp)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4099 /wd26812 /W4 /WX /experimental:external /external:anglebrackets /external:W0)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror -pedantic-errors)
endif()

