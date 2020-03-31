//
// Created by alexa on 3/25/2020.
//

#ifndef RENDER_HEADERS
#define RENDER_HEADERS

#include <vulkan/vulkan.hpp>
#include <stb/stb_image.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// On windows this includes vulkan.h which includes windows.h
#define NOMINMAX
#include <vk_mem_alloc.h>
#include <assimp/scene.h>
#include <assimp/DefaultIOStream.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <optional>
#include <set>
#include <chrono>
#include <string>
#include <fstream>
#include <utility>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>
#include <format>

#endif //RENDER_HEADERS
