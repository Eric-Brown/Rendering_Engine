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
#include <stdexcept>
#include <chrono>
#include "CommonIncludes.h"
#include "Vertex.h"

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class TriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow *window{};
	static const int WIDTH{800};
	static const int HEIGHT{600};

	void initWindow();

	void initVulkan() {
		initVulkanBeforePipeline();
		createGraphicsPipeline();
		initVulkanAfterPipeline();
	}

	void mainLoop();

	void cleanup();

	void initVulkanBeforePipeline();

	void createGraphicsPipeline() {
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		createGraphicsPipelineFromDescriptions(bindingDescription, attributeDescriptions);
	}

	void initVulkanAfterPipeline();

	VkInstance instance{};

	void createInstance();

	VkDebugUtilsMessengerEXT debugMessenger{};

	void setupDebugMessenger();

	VkSurfaceKHR surface{};

	void createSurface();

	VkPhysicalDevice physicalDevice{};

	void pickPhysicalDevice();

	VkDevice device{};

	void createLogicalDevice();

	VkSwapchainKHR swapChain;

	void createSwapChain();


	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	void createImageViews();

	VkRenderPass renderPass;

	void createRenderPass();

	VkDescriptorSetLayout descriptorSetLayout;

	void createDescriptorSetLayout();

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

	static inline const std::vector<const char *> requiredDeviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	static inline const std::vector<const char *> validationLayers{
			"VK_LAYER_KHRONOS_validation"
	};
	// Done so that validation can be toggled in the future
	//	static const bool enableValidationLayers = true;




	VkQueue graphicsQueue{};
	VkQueue presentQueue{};


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
	std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f,  -0.5f, 0.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f,  0.5f,  0.0f},  {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f,  0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f,  -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f,  0.5f,  -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f,  -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};
	std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
	};
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;


	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) const;


	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	bool isDeviceSuitable(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

	size_t scoreDevice(VkPhysicalDevice device);


	void createGraphicsPipelineFromDescriptions(VkVertexInputBindingDescription &bindingDescription,
	                                            std::array<VkVertexInputAttributeDescription, 3> &attributeDescriptions);


	void createFramebuffers();

	void createCommandPool();

	void cleanupSwapChain();

	void recreateSwapChain();

	void createCommandBuffers();

	void createSyncObjects();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
	                  VkDeviceMemory &bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void createVertexBuffer();

	void createIndexBuffer();


	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
	                 VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createTextureImage();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void createTextureImageView();

	void createTextureSampler();

	bool hasStencilComponent(VkFormat format);

	VkFormat findDepthFormat();


	VkFormat
	findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void createDepthResources();

	//moving to cpp messes up??




	VkApplicationInfo createApplicationInfo() const;

	VkInstanceCreateInfo createInstanceCreateInfo(VkApplicationInfo &appInfo);

	void validateExtensions(const std::vector<const char *> &toValidate) const;

	std::vector<const char *> getRequiredExtensions() const;

	void validateLayerSupport();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
	                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	                                                    void *pUserData);

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();


	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

	static std::vector<char> readFile(const std::string &filename);

	VkShaderModule createShaderModule(const std::vector<char> &code);
};


#endif //DNDIDEA_TRIANGLEAPPLICATION_H
