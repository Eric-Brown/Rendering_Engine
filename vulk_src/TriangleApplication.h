//
// Created by alexa on 5/18/2019.
//

#ifndef DNDIDEA_TRIANGLEAPPLICATION_H
#define DNDIDEA_TRIANGLEAPPLICATION_H

static const char *const COMMAND_POOL_FAIL_CREATE_MSG = "Failed to create command pool!";

static const char *const FRAMEBUFFER_CREATE_FAIL_MSG = "Failed to create framebuffer";

static const char *const WINDOW_SURF_CREATE_FAIL_MSG = "Failed to create window surface.";

static const char *const RENDER_PASS_CREATE_FAIL_MSG = "Failed to create render pass!";

static const char *const SWAP_CHAIN_CREATE_FAIL_MSG = "Failed to create swap chain!";

static const char *const LOGI_DEV_CREATE_FAIL_MSG = "Failed to create logical device!";

static const char *const NO_VULK_DEV_AVAILABLE_MSG = "No Vulkan devices available.";

static const char *const VULK_DEV_NOT_SUITABLE_MSG = "No suitable Vulkan devices available.";

static const char *const NO_SUITABLE_MEMORY_MSG = "Failed to find suitable memory type!";

static const char *const PIPELINE_LAYOUT_CREATE_FAIL_MSG = "Failed to create pipeline layout!";

static const char *const PIPELINE_CREATE_FAIL_MSG = "Failed to create graphics pipeline!";

static const char *const TEXTURE_FORMAT_NOT_SUPPORT_BLITTING_MSG = "Texture image format does not support linear blitting!";

#include <vector>
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <iterator>
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
	TriangleApplication();

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

	static inline constexpr std::array<const char *, 1> requiredDeviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	static inline constexpr std::array<const char *, 1> validationLayers{
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
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	uint32_t mipLevels; //for texture
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

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

	VkSampleCountFlagBits getMaxUsableSampleCount();

	void createFramebuffers();

	void createCommandPool();

	void cleanupSwapChain();

	void recreateSwapChain();

	void createCommandBuffers();

	void createSyncObjects();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
	                  VkDeviceMemory &bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
	                           uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void createVertexBuffer();

	void createIndexBuffer();


	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createColorResources();

	void
	createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
	            VkImageTiling tiling,
	            VkImageUsageFlags usage,
	            VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createTextureImage();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void createTextureImageView();

	void createTextureSampler();

	bool hasStencilComponent(VkFormat format);

	VkFormat findDepthFormat();

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


	VkFormat
	findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void createDepthResources();

	bool readModelFile(const std::string &pFile);

	void processSceneObject(const aiScene *scene);

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

	void cleanupImageResources() const;

	void cleanupPipelineResources() const;
};


#endif //DNDIDEA_TRIANGLEAPPLICATION_H
