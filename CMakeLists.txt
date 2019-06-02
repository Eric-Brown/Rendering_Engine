cmake_minimum_required(VERSION 3.10)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.23.165.tar.gz"
        SHA1 "5a73f91df5f6109c0bb1104d0c0ee423f7bece79"
#        FILEPATH "C:/Users/alexa/Documents/Programming_Files/CPP Projects/DnDIdea/cmake/Hunter/config.cmake"
)

project(DnDIdea)

cmake_policy(SET CMP0074 NEW)

find_package(OpenGL REQUIRED)

hunter_add_package(glfw)
find_package(glfw3 REQUIRED)

hunter_add_package(glm)
find_package(glm REQUIRED)

hunter_add_package(stb)
find_package(stb CONFIG REQUIRED)

#hunter_add_package(glbinding)
#find_package(glbinding CONFIG REQUIRED)

option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)

if(AUTO_LOCATE_VULKAN)
    message(STATUS "Attempting auto locate Vulkan using CMake......")

    # Find Vulkan Path using CMake's Vulkan Module
    # This will return Boolean 'Vulkan_FOUND' indicating the status of find as success(ON) or fail(OFF).
    # Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY' with required libraries.
    find_package(Vulkan)

    # Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
    if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
        set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
        STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    endif()

    if(NOT Vulkan_FOUND)
        # CMake may fail to locate the libraries but could be able to
        # provide some path in Vulkan SDK include directory variable
        # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
        message(STATUS "Failed to locate Vulkan SDK, retrying again...")
        if(EXISTS "${VULKAN_PATH}")
            message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
        else()
            message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
            message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
            return()
        endif()
    endif()
else()
    message(STATUS "Attempting to locate Vulkan SDK using manual path......")
    set(VULKAN_SDK "C:/VulkanSDK")
    set(VULKAN_VERSION "1.0.33.0")
    set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
    message(STATUS "Using manual specified path: ${VULKAN_PATH}")

    # Check if manual set path exists
    if(NOT EXISTS "${VULKAN_PATH}")
        message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.
		For more information on correct installation process, please refer to subsection 'Getting started with Lunar-G SDK'
		and 'Setting up first project with CMake' in Chapter 3, 'Shaking hands with the device' in this book 'Learning Vulkan', ISBN - 9781786469809.")
        return()
    endif()
endif()
# Include Vulkan header files from Vulkan SDK
include_directories(AFTER ${VULKAN_PATH}/Include)
set(VULKAN_LIB_LIST "vulkan-1")

# Link directory for vulkan-1
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
# Include Vulkan header files from Vulkan SDK
include_directories(AFTER ${VULKAN_PATH}/Include)

# Link directory for vulkan-1
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
set(CMAKE_CXX_STANDARD 17)

#add_subdirectory(src)
add_subdirectory(vulk_src)
