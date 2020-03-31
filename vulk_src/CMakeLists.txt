cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(Vulk_Main)

add_executable(${PROJECT_NAME} main.cpp
        ExternalHeaders.h
        Application.cpp Application.h
        Vertex.h Vertex.cpp
        Model.cpp Model.h
        VulkanMemoryManager.cpp VulkanMemoryManager.h
        VulkanImageManager.cpp VulkanImageManager.h
        )
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw glm Vulkan::Vulkan Assimp::assimp VulkanMemoryAllocator::VulkanMemoryAllocator)
if (MSVC)
    #    See PROJECT_ROOT CMakeLists.txt comment for why this is commented out.
    #    target_compile_options(${PROJECT_NAME} PUBLIC )
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror -pedantic-errors)
endif ()

