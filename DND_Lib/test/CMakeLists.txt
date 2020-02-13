project(DND_LIB_TEST)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} test_main.cpp catch.hpp CharacterTests.cpp)
