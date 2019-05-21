//
// Created by alexa on 5/18/2019.
//

#ifndef DNDIDEA_TRIANGLEAPPLICATION_H
#define DNDIDEA_TRIANGLEAPPLICATION_H

#include <vector>
#include <algorithm>
#include <iostream>
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

		bool isComplete() {
			return graphicsFamily.has_value();
		}
	};

	GLFWwindow *window{};
	static const int WIDTH{800};
	static const int HEIGHT{600};
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
		for (const auto &queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
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

	bool isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		// Only caring about having a geometry shader right now
		QueueFamilyIndices indices = findQueueFamilies(device);
		return static_cast<bool>(indices.isComplete() && deviceFeatures.geometryShader);
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
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		// Can add check here to see if validation is enabled. Not really used, but good for legacy support
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}
		// 0 is the Queue index. Since creating only one, it is zero.
		vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void createInstance() {
		validateLayerSupport();
		VkApplicationInfo applicationInfo{createApplicationInfo()};
		// Creation method intentionally lacks extension info
		VkInstanceCreateInfo createInfo{createInstanceCreateInfo(applicationInfo)};
		auto extensions = getRequiredExtensions();
		validateExtensions(extensions);
		createInfo.enabledExtensionCount = extensions.size();
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
		info.enabledLayerCount = validationLayers.size();
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
		vkDestroyDevice(logicalDevice, nullptr);
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
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
