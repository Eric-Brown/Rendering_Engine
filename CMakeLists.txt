cmake_minimum_required(VERSION 3.10)

# Set values for OpenGL, CPP and optionally a policy
set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE "GLVND")
#set(HUNTER_STATUS_DEBUG ON)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    cmake_policy(SET CMP0074 NEW)
endif ()
# Set value to suppress the CMake warnings that vcpkg will emit while in VS2019
set(VCPKG_SUPPRESS_INSTALLED_LIBRARIES_WARNING TRUE)
set(_NO_DEBUG_HEAP 1)
set(_ITERATOR_DEBUG_LEVEL 0)

# Import in HunterGate file for using Hunter
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.251.tar.gz"
        SHA1 "5659b15dc0884d4b03dbd95710e6a1fa0fc3258d"
)

project(DnDIdea)

#set(HUNTER_PACKAGES glfw Assimp glm stb)
#include(FetchContent)
#FetchContent_Declare(SetupHunter GIT_REPOSITORY https://github.com/cpp-pm/gate)
#FetchContent_MakeAvailable(SetupHunter)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
hunter_add_package(glfw)
find_package(glfw3 REQUIRED)
hunter_add_package(stb)
find_package(stb REQUIRED)
hunter_add_package(glm)
find_package(glm REQUIRED)
hunter_add_package(Assimp)
find_package(Assimp CONFIG REQUIRED)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/wd4099 /experimental:external /external:anglebrackets /external:W0 /W4 /WX /EHsc")
else ()
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Wextra -pedantic -Werror")
endif ()

#add_subdirectory(src)
add_subdirectory(vulk_src)
#add_subdirectory(DND_Lib)
