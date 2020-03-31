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

#include "VulkanMemoryManager.h"
#include "VulkanImageManager.h"
#include "Vertex.h"
#include "Model.h"

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class Application
{
private:
	//Constants
	static inline constexpr std::array<const char *, 1> requiredDeviceExtensions{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	static inline constexpr std::array<const char *, 1> validationLayers{
		"VK_LAYER_KHRONOS_validation"};
	static const int WIDTH{800};
	static const int HEIGHT{600};
	static const int MAX_FRAMES_IN_FLIGHT = 2;
	//Instance
	GLFWwindow *window{};
	vk::Instance instance{};
	vk::SurfaceKHR surface{};
	vk::PhysicalDevice physicalDevice{};
	vk::Device device{};
	vk::DebugUtilsMessengerEXT debugMessenger{};
	vk::SwapchainKHR swapChain{};
	std::vector<vk::Image> swapChainImages{};
	vk::Format swapChainImageFormat{};
	vk::Extent2D swapChainExtent{};
	std::vector<vk::ImageView> swapChainImageViews{};
	vk::RenderPass renderPass{};
	vk::DescriptorSetLayout descriptorSetLayout{};
	vk::PipelineLayout pipelineLayout{};
	vk::Pipeline graphicsPipeline{};
	vk::Queue graphicsQueue{};
	vk::Queue presentQueue{};
	std::vector<vk::Framebuffer> swapChainFramebuffers{};
	vk::CommandPool commandPool{};
	std::vector<vk::CommandBuffer> commandBuffers{};
	std::vector<vk::Semaphore> imageAvailableSemaphores{};
	std::vector<vk::Semaphore> renderFinishedSemaphores{};
	std::vector<vk::Fence> inFlightFences{};
	size_t currentFrame = 0;
	bool framebufferResized{false};
	vk::Image depthImage{};
	VmaAllocation depthImageMemory{};
	vk::ImageView depthImageView{};
	vk::Image colorImage{};
	VmaAllocation colorImageMemory{};
	vk::ImageView colorImageView{};
	vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
	std::vector<vk::Buffer> uniformBuffers{};
	std::vector<VmaAllocation> uniformBuffersAllocations{};
	vk::DescriptorPool descriptorPool{};
	// Done so that validation can be toggled in the future
	//	static const bool enableValidationLayers = true;
	std::vector<vk::DescriptorSet> descriptorSets{};
	std::vector<std::unique_ptr<Model>> modelHandles{};

public:
	Application();

	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities{};
		std::vector<vk::SurfaceFormatKHR> formats{};
		std::vector<vk::PresentModeKHR> presentModes{};

		SwapChainSupportDetails(vk::SurfaceCapabilitiesKHR capabilitiesKHR,
								std::vector<vk::SurfaceFormatKHR> surfaceFormats,
								std::vector<vk::PresentModeKHR> presentModesKHR)
			: capabilities{std::move(capabilitiesKHR)},
			  formats{std::move(surfaceFormats)},
			  presentModes{std::move(presentModesKHR)}
		{
		}
	};

	void initWindow();

	void initVulkan()
	{
		initVulkanBeforePipeline();
		createGraphicsPipeline();
		initVulkanAfterPipeline();
	}

	void mainLoop();

	void cleanup();

	void loadHandles();

	void initVulkanBeforePipeline();

	void createGraphicsPipeline()
	{
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		createGraphicsPipelineFromDescriptions(bindingDescription, attributeDescriptions);
	}

	void initGlobalVmaAllocator();

	void initVulkanAfterPipeline();

	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	void createRenderPass();

	void createDescriptorSetLayout();

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

	void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) const;

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

	bool isDeviceSuitable(vk::PhysicalDevice device);

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

	size_t scoreDevice(vk::PhysicalDevice device);

	void createGraphicsPipelineFromDescriptions(vk::VertexInputBindingDescription &bindingDescription,
												std::array<vk::VertexInputAttributeDescription, 3> &attributeDescriptions);

	vk::SampleCountFlagBits getMaxUsableSampleCount();

	void createFramebuffers();

	void destroyGlobalAllocator();

	void createCommandPool();

	void cleanupSwapChain();

	void recreateSwapChain();

	void createCommandBuffers();

	void createSyncObjects();

	void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
							   uint32_t mipLevels);

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createColorResources();

	void
	createImage(uint32_t width, uint32_t height, uint32_t imageMipLevels, vk::SampleCountFlagBits numSamples,
				vk::Format format,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
				VmaMemoryUsage memUsage, vk::Image &image, VmaAllocation &imageMemory);

	vk::CommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

	vk::ImageView
	createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);

	bool hasStencilComponent(vk::Format format);

	vk::Format findDepthFormat();

	void
	generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	vk::Format
	findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
						vk::FormatFeatureFlags features);

	void createDepthResources();

	vk::ApplicationInfo createApplicationInfo() const;

	vk::InstanceCreateInfo createInstanceCreateInfo(vk::ApplicationInfo &appInfo);

	void validateExtensions(const std::vector<const char *> &toValidate) const;

	[[nodiscard]] std::vector<const char *> getRequiredExtensions() const;

	void validateLayerSupport();

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
														  VkDebugUtilsMessageTypeFlagsEXT messageType,
														  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
														  void *pUserData);

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

	static std::vector<char> readFile(const std::string &filename);

	vk::ShaderModule createShaderModule(const std::vector<char> &code);

	void cleanupImageResources() const;

	std::tuple<vk::Buffer, VmaAllocation> initializeStagingBuffer(void *data, vk::DeviceSize dataSize);

	void cleanupPipelineResources() const;
};

#endif //DNDIDEA_TRIANGLEAPPLICATION_H
