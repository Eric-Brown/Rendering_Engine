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

class triangleapplication {

private:
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
			VkExtent2D actualExtent = {WIDTH, HEIGHT};
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

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
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

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(logicalDevice, imageView, nullptr);
		}
		vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
		vkDestroyDevice(logicalDevice, nullptr);
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
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
