cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0074 NEW)

set(OpenGL_GL_PREFERENCE "GLVND")

set(CMAKE_CXX_STANDARD 17)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.251.tar.gz"
        SHA1 "5659b15dc0884d4b03dbd95710e6a1fa0fc3258d"
)

        #        LOCAL `${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/config.cmake`

set(HUNTER_PACKAGES glfw Assimp glm stb)

include(FetchContent)
FetchContent_Declare(SetupHunter GIT_REPOSITORY https://github.com/cpp-pm/gate)
FetchContent_MakeAvailable(SetupHunter)

project(DnDIdea)

find_package(OpenGL REQUIRED)
hunter_add_package(glfw)

find_package(glfw3 REQUIRED)
hunter_add_package(glm)
find_package(glm REQUIRED)

find_package(stb CONFIG REQUIRED)
hunter_add_package(Assimp)
find_package(Assimp CONFIG REQUIRED)

find_package(Vulkan REQUIRED)


if (MSVC)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/W4 /WX")
else ()
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Wextra -pedantic")
endif ()
#add_subdirectory(src)
add_subdirectory(vulk_src)
#add_subdirectory(DND_Lib)
