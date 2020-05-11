# Basic Rendering Engine

This is a learning project for building a rendering engine using the Vulkan API.
The project originally started by following along with [Vulkan Tutorial](https://vulkan-tutorial.com/) and has branched off once that tutorial ended.
The intentions for this project are to learn graphical computing concepts whilst following along with the book [Real-Time Rendering](http://www.realtimerendering.com/).

Notes on the learned concepts are in the "review_notes" folder inside of the "vulk_src" folder.
The notes are neither comprehensive, nor particularly organized.
They are also from information gathered from many sources, not necessarly just the tutorial and/or the book.

The [Issues](https://github.com/Eric-Brown/Rendering_Engine/issues) tab is where work is currently being organized.
GitLFS is being used to manage the larger resources like model files, materials, and textures.

## Building Requirements

The project is CMake based, so you will need a minimum version of CMake 3.10.
Additionally, the project is C++17, so you will need a C++17 compatible compiler.
The CMake package manager project [Hunter](https://github.com/cpp-pm/hunter) is used to bring in dependencies.
Hunter will download and compile dependencies.
If the **HUNTER_ROOT** environmental variable is not set, Hunter will put the dependencies in a default location.
While most dependencies will be taken care of by Hunter, it is still required that you have both Vulkan compatible drivers, and the [Vulkan SDK](https://vulkan.lunarg.com/).

NOTE:
Current iteration of the project requires GLFW 3.4 which is NOT a release version.
The reasoning is that GLFW 3.3 has a race condition bug on linux which causes a crash on window close.
If you are trying to build from source, you will either need to manually compile the GLFW master branch, or revert back to 3.3 in the CMakeLists.txt file.
