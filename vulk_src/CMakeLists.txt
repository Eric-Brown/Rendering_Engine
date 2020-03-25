cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(Vulk_Main)
#if (MSVC)
#    /W4 /WX
#endif ()

add_executable(${PROJECT_NAME} main.cpp
        TriangleApplication.cpp
        TriangleApplication.h
        Vertex.h Vertex.cpp
        ExternalIncludes.h
        Model.cpp Model.h)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm Vulkan::Vulkan Assimp::assimp)
if (MSVC)
#    target_compile_options(${PROJECT_NAME} PUBLIC )
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror -pedantic-errors)
endif ()

