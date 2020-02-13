project(DND_LIB)
set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} Character.cpp ../test/test_main.cpp InitiativeList.h)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR})

