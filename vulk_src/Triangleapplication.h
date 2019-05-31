//
// Created by alexa on 5/18/2019.
//

#ifndef DNDIDEA_TRIANGLEAPPLICATION_H
#define DNDIDEA_TRIANGLEAPPLICATION_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include <array>
#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stdexcept>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
	                                                                        "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
};

class triangleapplication {

private:
	static const int MAX_FRAMES_IN_FLIGHT = 2;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	GLFWwindow *window{};
	static const int WIDTH{800};
	static const int HEIGHT{600};
	static inline const std::vector<const char *> requiredDeviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	static inline const std::vector<const char *> validationLayers{
			"VK_LAYER_KHRONOS_validation"
	};
	// Done so that validation can be toggled in the future
	//	static const bool enableValidationLayers = true;
	VkInstance instance{};
	VkDebugUtilsMessengerEXT debugMessenger{};
	VkPhysicalDevice physicalDevice{};
	VkDevice logicalDevice{};
	VkQueue graphicsQueue{};
	VkQueue presentQueue{};
	VkSurfaceKHR surface{};
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;
	bool framebufferResized{false};
	const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};
	const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
	};
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	void setupDebugMessenger() {
		//If no validation requested, just do nothing
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		int i = 0;
		VkBool32 presentSupport{false};
		for (const auto &queueFamily : queueFamilies) {
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queueFamily.queueCount > 0) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
				}
				if (presentSupport) {
					indices.presentFamily = i;
				}
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) const {
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = debugCallback;
		info.pUserData = nullptr; // Optional
		createInfo = info;
	}

	void pickPhysicalDevice() {
		using namespace std;
		uint32_t deviceCount{};
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) throw runtime_error("No Vulkan devices available.");
		vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		devices.erase(find_if(devices.begin(), devices.end(),
		                      [&](VkPhysicalDevice &dev) { return !isDeviceSuitable(dev); }), devices.end());
		if (devices.empty()) throw runtime_error("No suitable Vulkan devices available.");
		sort(devices.begin(), devices.end(), [&](VkPhysicalDevice &a, VkPhysicalDevice &b) {
			return scoreDevice(a) > scoreDevice(b);
		});
		physicalDevice = devices.front();
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		using namespace std;
		uint32_t extCount{};
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
		vector<VkExtensionProperties> availableExtensions(extCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExtensions.data());
		set<string> requiredExtensions{requiredDeviceExtensions.begin(), requiredDeviceExtensions.end()};
		for (const auto &extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		// Only caring about having a geometry shader right now
		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		bool swapChainSuitable = false;
		if (extensionsSupported) {
			auto supportDetails = querySwapChainSupport(device);
			swapChainSuitable = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
		}
		return static_cast<bool>(indices.isComplete() && extensionsSupported && deviceFeatures.geometryShader &&
		                         swapChainSuitable);
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
		using namespace std;
		//surface has no preference...good
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}
		auto found = find_if(availableFormats.begin(), availableFormats.end(), [&](const VkSurfaceFormatKHR &val) {
			return val.format == VK_FORMAT_B8G8R8A8_UNORM && val.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		});
		if (found != availableFormats.end()) return *found;
		// if we don't get what we want, then just go with first entry
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
		using namespace std;
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
		auto found = for_each(availablePresentModes.begin(), availablePresentModes.end(),
		                      [&](const VkPresentModeKHR &val) {
			                      if (val == VK_PRESENT_MODE_MAILBOX_KHR) { bestMode = val; }
			                      else if (bestMode == VK_PRESENT_MODE_FIFO_KHR &&
			                               val == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				                      bestMode = val;
			                      }
		                      });
		return bestMode;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t >(height)};
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
			                                capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
			                                 capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	size_t scoreDevice(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		size_t score{};
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
		score += deviceProperties.limits.maxImageDimension2D;
		// and so on...
		return score;
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		std::set<uint32_t> uniqueQueueFamilies{indices.graphicsFamily.value(), indices.presentFamily.value()};
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		float queuePriority = 1.0f;
		for (auto &family : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueFamilyIndex = family;
			info.queueCount = 1;
			info.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(info);
		}
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t >(queueCreateInfos.size());
		VkPhysicalDeviceFeatures deviceFeatures = {};
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t >(requiredDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		// Can add check here to see if validation is enabled. Not really used, but good for legacy support
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}
		// 0 is the Queue index. Since creating only one, it is zero.
		vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
	}

	void createSurface() {
		using namespace std;
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw runtime_error("Failed to create window surface.");
		}
	}

	void createSwapChain() {
		auto swapChainSupport = querySwapChainSupport(physicalDevice);
		auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		auto extent = chooseSwapExtent(swapChainSupport.capabilities);
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
		    imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}
		vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void createGraphicsPipeline() {
		auto vert{readFile("shaders/sprvs/vertex_buffer_vert.spv")};
		auto frag{readFile("shaders/sprvs/triangle_application_frag.spv")};
		auto verMod{createShaderModule(vert)};
		auto fraMod{createShaderModule(frag)};
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = verMod;
		vertShaderStageInfo.pName = "main"; //entrypoint name
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fraMod;
		fragShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo stages[] = {vertShaderStageInfo, fragShaderStageInfo};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) swapChainExtent.width;
		viewport.height = (float) swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = swapChainExtent;
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) !=
		    VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		vkDestroyShaderModule(logicalDevice, verMod, nullptr);
		vkDestroyShaderModule(logicalDevice, fraMod, nullptr);
	}

	void createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (auto i{0}; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
					swapChainImageViews[i]
			};
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) !=
			    VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer");
			}
		}
	}

	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; //optional
		if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void cleanupSwapChain() {
		for (auto swapChainFramebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(logicalDevice, swapChainFramebuffer, nullptr);
		}
		vkFreeCommandBuffers(logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()),
		                     commandBuffers.data());
		vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
		for (auto swapChainImageView: swapChainImageViews) {
			vkDestroyImageView(logicalDevice, swapChainImageView, nullptr);
		}
		vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
	}

	void recreateSwapChain() {
		int width{}, height{};
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(logicalDevice);
		cleanupSwapChain();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandBuffers();
	}

	void createCommandBuffers() {
		commandBuffers.resize(swapChainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional
			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapChainExtent;
			VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			VkBuffer vertexBuffers[] = {vertexBuffer};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}


		}

	}

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			    vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			    vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
	                  VkDeviceMemory &bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}
		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);
		vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
	}

	void createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		             stagingBufferMemory);

		void *data;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t) bufferSize);
		vkUnmapMemory(logicalDevice, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
	}


	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t) bufferSize);
		vkUnmapMemory(logicalDevice, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();
		createCommandBuffers();
		createSyncObjects();
	}

	void createInstance() {
		validateLayerSupport();
		VkApplicationInfo applicationInfo{createApplicationInfo()};
		// Creation method intentionally lacks extension info
		VkInstanceCreateInfo createInfo{createInstanceCreateInfo(applicationInfo)};
		auto extensions = getRequiredExtensions();
		validateExtensions(extensions);
		createInfo.enabledExtensionCount = static_cast<uint32_t >(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan instance.");
		}

	}

	VkApplicationInfo createApplicationInfo() const {
		VkApplicationInfo info{};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pApplicationName = "Hello Triangle";
		info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		info.pEngineName = "No Engine";
		info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		info.apiVersion = VK_API_VERSION_1_0;
		return info;
	}

	VkInstanceCreateInfo createInstanceCreateInfo(VkApplicationInfo &appInfo) {
		VkInstanceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &appInfo;
		// In future can add check if validation is requested
		info.enabledLayerCount = static_cast<uint32_t >( validationLayers.size());
		info.ppEnabledLayerNames = validationLayers.data();

		return info;
	}

	void validateExtensions(const std::vector<const char *> &toValidate) const {
		using namespace std;
		// Get all supported extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		for (auto name : toValidate) {
			string toFind{name};
			auto found = find_if(extensions.begin(), extensions.end(), [&](const VkExtensionProperties &props) {
				string prop_name{props.extensionName};
				return prop_name == toFind;
			});
			if (found == extensions.end()) {
				throw runtime_error(string("Extension:") + toFind + " is not supported.");
			}
		}
	}

	std::vector<const char *> getRequiredExtensions() const {
		using namespace std;
		// Get GLFW needed extensions
		uint32_t glfwExtensionCount{};
		const char **glfwExtensions{};
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		vector<const char *> allExtensions{glfwExtensions, glfwExtensions + glfwExtensionCount};
		// In future can check if validation is requested
		allExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return allExtensions;
	}

	void validateLayerSupport() {
		using namespace std;
		uint32_t layerCount{};
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (auto layerName: validationLayers) {
			string toFind{layerName};
			auto found = find_if(availableLayers.begin(), availableLayers.end(), [&](const VkLayerProperties &props) {
				string currName{props.layerName};
				return currName == toFind;
			});
			if (found == availableLayers.end()) {
				throw runtime_error("Requested validation layer does not exist.");
			}
		}
	}

	static VKAPI_ATTR VkBool32

	VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData
	) {
		using namespace std;
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			cerr << "Validation layer: " << pCallbackData->pMessage << endl;
		}
		// Only true if wanting the call to be aborted...generally only used for testing Validation Layers
		return VK_FALSE;
	}

	void drawFrame() {
		using namespace std;
		vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		uint32_t imageIndex;
		auto result = vkAcquireNextImageKHR(logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(),
		                                    imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw runtime_error("failed to acquire swap chain image");
		}
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = {swapChain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw runtime_error("failed to present swap chain image");
		}
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(logicalDevice);
	}

	void cleanup() {
		cleanupSwapChain();

		vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
		vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
		vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
		vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		vkDestroyDevice(logicalDevice, nullptr);
		// Can add logic to test if we are debugging or not
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		glfwSetWindowUserPointer(window, this);
	}

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
		auto app = reinterpret_cast<triangleapplication * >(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	static std::vector<char> readFile(const std::string &filename) {
		using namespace std;
		ifstream file(filename, ios::ate | ios::binary);
		if (!file) {
			throw runtime_error("Could not open file: " + filename);
		}
		auto f_Size{file.tellg()};
		vector<char> buffer(f_Size);
		file.seekg(0);
		file.read(buffer.data(), f_Size);
		file.close();
		assert(buffer.size() == f_Size);
		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char> &code) {
		using namespace std;
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
		VkShaderModule module;
		if (vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &module) != VK_SUCCESS) {
			throw runtime_error("Uh oh spaghettio, no shader module created.");
		}
		return module;
	}

public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
};


#endif //DNDIDEA_TRIANGLEAPPLICATION_H
