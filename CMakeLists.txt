cmake_minimum_required(VERSION 3.10)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.23.165.tar.gz"
        SHA1 "5a73f91df5f6109c0bb1104d0c0ee423f7bece79"
#        FILEPATH "C:/Users/alexa/Documents/Programming_Files/CPP Projects/DnDIdea/cmake/Hunter/config.cmake"
)

project(DnDIdea)

find_package(OpenGL REQUIRED)

hunter_add_package(glfw)
find_package(glfw3 REQUIRED)

hunter_add_package(glm)
find_package(glm REQUIRED)

hunter_add_package(glbinding)
find_package(glbinding CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
