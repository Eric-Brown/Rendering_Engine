//
// Created by alexa on 5/18/2019.
//

#include "Application.h"

bool Application::readModelFile(const std::string &pFile) {
	// Create an instance of the Importer class
	Assimp::Importer importer;

	importer.SetPropertyBool("GLOB_MEASURE_TIME", true);
	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);
	std::cout << "Reading file now...\n" << std::endl;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene *scene = importer.ReadFile(pFile,
	                                         aiProcess_ValidateDataStructure | // Validates imported structure
	                                         //	                                         aiProcess_MakeLeftHanded |
	                                         //	                                         	                                         aiProcess_FlipWindingOrder |
	                                         //                                             aiProcess_PreTransformVertices |
	                                         aiProcess_RemoveRedundantMaterials | // Removes duplicated materials
	                                         //	                                         aiProcess_FindInstances |
	                                         //	                                         aiProcess_RemoveComponent |
	                                         aiProcess_FindDegenerates |
	                                         //	                                         aiProcess_GenUVCoords |
	                                         aiProcess_Triangulate |
	                                         aiProcess_FlipUVs |
	                                         //                                             aiProcess_FindInvalidData |
	                                         //                                             aiProcess_FixInfacingNormals |
	                                         //                                             aiProcess_SplitLargeMeshes |
	                                         //                                             aiProcess_SortByPType |
	                                         //???
	                                         //	                                         aiProcess_GenNormals |
	                                         //	                                         aiProcess_CalcTangentSpace |
	                                         //	                                         aiProcess_OptimizeMeshes |
	                                         //                                                                                          aiProcess_JoinIdenticalVertices |
	                                         //	                                         aiProcess_LimitBoneWeights |
	                                         //	                                         aiProcess_ImproveCacheLocality |
	                                         //                                             aiProcess_CalcTangentSpace |

	                                         aiProcess_JoinIdenticalVertices |
	                                         //	                                         aiProcess_SplitLargeMeshes |
	                                         //	                                                                                      aiProcess_SortByPType |
	                                         //                                             aiProcess_FlipUVs |
	                                         //                                             aiProcess_FlipWindingOrder |
	                                         0
	);

	// If the import failed, report it
	if (!scene) {
		std::cerr << "Could not read file" << std::endl;
//		DoTheErrorLogging( importer.GetErrorString());
		return false;
	}
	Assimp::DefaultLogger::kill();
	std::cout << "Processing Geometry now...\n" << std::endl;

	// Now we can access the file's contents.
	processSceneObject(scene);

	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}

void Application::setupDebugMessenger() {
	//If no validation requested, just do nothing
	vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);
	vk::DynamicLoader dl;
	auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>(
			"vkGetInstanceProcAddr");
	vk::DispatchLoaderDynamic dispatchLoaderDynamic{instance, vkGetInstanceProcAddr};
	debugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dispatchLoaderDynamic);
}

void Application::cleanup() {
	cleanupSwapChain();
	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);
	vmaDestroyImage(globalAllocator, textureImage, textureImageMemory);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vmaDestroyBuffer(globalAllocator, indexBuffer, indexBufferAllocation);
	vmaDestroyBuffer(globalAllocator, vertexBuffer, vertexBufferAllocation);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		device.destroySemaphore(renderFinishedSemaphores[i]);
		device.destroySemaphore(imageAvailableSemaphores[i]);
		device.destroyFence(inFlightFences[i]);
	}
	device.destroyCommandPool(commandPool);
	destroyGlobalAllocator();
	vkDestroyDevice(device, nullptr);
	// Can add logic to test if we are debugging or not
	vk::DynamicLoader dl;
	auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>(
			"vkGetInstanceProcAddr");
	vk::DispatchLoaderDynamic dispatchLoaderDynamic{instance, vkGetInstanceProcAddr};
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dispatchLoaderDynamic);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::drawFrame() {
	using namespace std;
	device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, (numeric_limits<uint64_t>::max)());
	uint32_t imageIndex;
	auto result = device.acquireNextImageKHR(swapChain, (numeric_limits<uint64_t>::max)(),
	                                         imageAvailableSemaphores[currentFrame],
	                                         nullptr, &imageIndex);
	if (result == vk::Result::eErrorOutOfDateKHR) {
		recreateSwapChain();
		return;
	} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
		throw runtime_error("failed to acquire swap chain image");
	}
	updateUniformBuffer(imageIndex);
	vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1, &commandBuffers[imageIndex], 1, signalSemaphores);
	device.resetFences(1, &inFlightFences[currentFrame]);
	if (graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	vk::SwapchainKHR swapChains[] = {swapChain};
	vk::PresentInfoKHR presentInfo(1, signalSemaphores, 1, swapChains, &imageIndex, {});
	result = presentQueue.presentKHR(&presentInfo);
	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	} else if (result != vk::Result::eSuccess) {
		throw runtime_error("failed to present swap chain image");
	}
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Application::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(device);
}

void Application::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetWindowUserPointer(window, this);
}

vk::ShaderModule Application::createShaderModule(const std::vector<char> &code) {
	using namespace std;
	vk::ShaderModuleCreateInfo shaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t *>(code.data()));
	vk::ShaderModule module;
	if (device.createShaderModule(&shaderModuleCreateInfo, nullptr, &module) != vk::Result::eSuccess) {
		throw runtime_error("Uh oh spaghettio, no shader module created.");
	}
	return module;
}

std::vector<char> Application::readFile(const std::string &filename) {
	using namespace std;
	ifstream file(filename, ios::ate | ios::binary);
	if (!file) {
		throw runtime_error("Could not open file: " + filename);
	}
	auto f_Size{static_cast<unsigned long>(file.tellg())};
	vector<char> buffer(f_Size);
	file.seekg(0);
	file.read(buffer.data(), f_Size);
	file.close();
	assert(buffer.size() == f_Size);
	return buffer;
}

void Application::framebufferResizeCallback(GLFWwindow *window, int, int) {
	auto app = reinterpret_cast<Application * >(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Application::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * 0.5f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(90.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f,
	                            40.0f);
	ubo.proj[1][1] *= -1;
	void *data;
	vmaMapMemory(globalAllocator, uniformBuffersAllocations[currentImage], &data);
	memcpy(data, &ubo, sizeof(ubo));
	vmaUnmapMemory(globalAllocator, uniformBuffersAllocations[currentImage]);
}

vk::Bool32 Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                      VkDebugUtilsMessageTypeFlagsEXT,
                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                      void *) {
	using namespace std;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		cerr << "Validation layer: " << pCallbackData->pMessage << endl;
	}
	// Only true if wanting the call to be aborted...generally only used for testing Validation Layers
	return VK_FALSE;
}

void Application::validateLayerSupport() {
	using namespace std;
	auto layerProperties = vk::enumerateInstanceLayerProperties();
	for (auto layerName: validationLayers) {
		if (std::find_if(layerProperties.begin(), layerProperties.end(), [&](const vk::LayerProperties &props) {
			return strcmp(layerName, props.layerName) == 0;
		}) == layerProperties.end()) {
			throw runtime_error("Requested validation layer does not exist.");
		}
	}
}

std::vector<const char *> Application::getRequiredExtensions() const {
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

void Application::validateExtensions(const std::vector<const char *> &toValidate) const {
	using namespace std;
	// Get all supported extensions
	vector<vk::ExtensionProperties> extensions{vk::enumerateInstanceExtensionProperties()};
	for (auto name : toValidate) {
		string toFind{name};
		auto found = find_if(extensions.begin(), extensions.end(), [&](const vk::ExtensionProperties &props) {
			string prop_name{props.extensionName};
			return prop_name == toFind;
		});
		if (found == extensions.end()) {
			throw runtime_error(string("Extension:") + toFind + " is not supported.");
		}
	}
}

vk::InstanceCreateInfo Application::createInstanceCreateInfo(vk::ApplicationInfo &appInfo) {
	vk::InstanceCreateInfo info({}, &appInfo, static_cast<uint32_t >(validationLayers.size()), validationLayers.data(),
	                            {}, {});
	return info;
}

vk::ApplicationInfo Application::createApplicationInfo() const {
	vk::ApplicationInfo info("Hello World", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0),
	                         VK_API_VERSION_1_0);
	return info;
}

void Application::createInstance() {
	validateLayerSupport();
	vk::ApplicationInfo applicationInfo{createApplicationInfo()};
	// Creation method intentionally lacks extension info
	vk::InstanceCreateInfo createInfo{createInstanceCreateInfo(applicationInfo)};
	auto extensions = getRequiredExtensions();
	validateExtensions(extensions);
	createInfo.enabledExtensionCount = static_cast<uint32_t >(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	populateDebugMessengerCreateInfo(debugCreateInfo);
	createInfo.pNext = &debugCreateInfo;
	vk::Result result = vk::createInstance(&createInfo, nullptr, &instance);
	if (result != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create Vulkan instance.");
	}
}

void Application::createDepthResources() {
	vk::Format depthFormat = findDepthFormat();
	createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, vk::ImageTiling::eOptimal,
	            vk::ImageUsageFlagBits::eDepthStencilAttachment, VMA_MEMORY_USAGE_GPU_ONLY, depthImage,
	            depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
	transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
}

vk::Format Application::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                            vk::FormatFeatureFlags features) {
	for (vk::Format format : candidates) {
		vk::FormatProperties props;
		physicalDevice.getFormatProperties(format, &props);
		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

vk::Format Application::findDepthFormat() {
	return findSupportedFormat(
			{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
			vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

bool Application::hasStencilComponent(vk::Format format) {
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void Application::createTextureSampler() {
	vk::SamplerCreateInfo samplerInfo{{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
	                                  vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
	                                  vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 16.0f, VK_FALSE, {}, 0.0f,
	                                  static_cast<float>(mipLevels), vk::BorderColor::eIntOpaqueBlack, VK_FALSE};
	textureSampler = device.createSampler(samplerInfo);
}

void Application::createTextureImageView() {
	textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,
	                                   mipLevels);
}

vk::ImageView Application::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags,
                                           uint32_t inMipLevels) {
	vk::ImageSubresourceRange subresourceRange{aspectFlags, 0, inMipLevels, 0, 1};
	vk::ImageViewCreateInfo viewInfo{{}, image, vk::ImageViewType::e2D, format, {}, subresourceRange};
	return device.createImageView(viewInfo);
}

void Application::createTextureImage() {
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load("../resources/textures/chalet.jpg", &texWidth, &texHeight,
	                            &texChannels,
	                            STBI_rgb_alpha);
	mipLevels = static_cast<uint32_t>(std::floor(std::log2((std::max)(texWidth, texHeight)))) + 1;
	auto imageSize = static_cast<vk::DeviceSize>(static_cast<vk::DeviceSize>(texWidth) * texHeight * 4);
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
	auto[stagingBuffer, stagingBufferMemory] = initializeStagingBuffer(pixels, imageSize);
	stbi_image_free(pixels);
	createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), mipLevels,
	            vk::SampleCountFlagBits::e1,
	            vk::Format::eR8G8B8A8Unorm,
	            vk::ImageTiling::eOptimal,
	            vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
	            vk::ImageUsageFlagBits::eSampled,
	            VMA_MEMORY_USAGE_GPU_ONLY, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eTransferDstOptimal, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	generateMipmaps(textureImage, vk::Format::eR8G8B8A8Unorm, texWidth, texHeight, mipLevels);
	VulkanMemoryManager::getInstance()->DestroyBuffer(stagingBuffer, stagingBufferMemory);
}

void Application::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
	commandBuffer.end();
	vk::SubmitInfo submitInfo{{}, {}, {}, 1, &commandBuffer};
	graphicsQueue.submit(1, &submitInfo, {});
	graphicsQueue.waitIdle();
	device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

vk::CommandBuffer Application::beginSingleTimeCommands() {
	vk::CommandBufferAllocateInfo allocInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};
	auto buffers{device.allocateCommandBuffers(allocInfo)};
	vk::CommandBuffer commandBuffer{buffers[0]};
	vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
	commandBuffer.begin(beginInfo);
	return commandBuffer;
}

void
Application::createImage(uint32_t width, uint32_t height, uint32_t imageMipLevels,
                         vk::SampleCountFlagBits numSamples,
                         vk::Format format,
                         vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage, VmaMemoryUsage memUsage, vk::Image &image,
                         VmaAllocation &imageMemory) {
	vk::ImageCreateInfo imageInfo{{}, vk::ImageType::e2D, format, vk::Extent3D{width, height, 1}, imageMipLevels, 1,
	                              numSamples, tiling, usage, vk::SharingMode::eExclusive, {}, {}, {}};
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = memUsage;
	auto[retImg, imgAlloc] = VulkanMemoryManager::getInstance()->createImage(imageInfo, allocationCreateInfo);
	image = retImg;
	imageMemory = imgAlloc;
}

void Application::createDescriptorSets() {
	std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo{descriptorPool, static_cast<uint32_t>(swapChainImages.size()),
	                                        layouts.data()};
	descriptorSets = device.allocateDescriptorSets(allocInfo);
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vk::DescriptorBufferInfo bufferInfo{uniformBuffers[i], 0, sizeof(UniformBufferObject)};
		vk::DescriptorImageInfo imageInfo{textureSampler, textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal};
		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
				vk::WriteDescriptorSet{descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, {}, &bufferInfo},
				vk::WriteDescriptorSet{descriptorSets[i], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler,
				                       &imageInfo}
		};
		device.updateDescriptorSets(static_cast<uint32_t >(descriptorWrites.size()), descriptorWrites.data(), 0,
		                            nullptr);
	}
}

void Application::createDescriptorPool() {
	vk::DescriptorPoolSize uboSize{vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(swapChainImages.size())};
	vk::DescriptorPoolSize samplerSize = {vk::DescriptorType::eCombinedImageSampler,
	                                      static_cast<uint32_t>(swapChainImages.size())};
	std::vector<vk::DescriptorPoolSize> poolSizes{uboSize, samplerSize};
	vk::DescriptorPoolCreateInfo poolInfo{{}, static_cast<uint32_t>(swapChainImages.size()),
	                                      static_cast<uint32_t>(poolSizes.size()), poolSizes.data()};
	descriptorPool = device.createDescriptorPool(poolInfo);
}

void Application::createUniformBuffers() {
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersAllocations.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VulkanMemoryManager::getInstance()
				->createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_ONLY,
				               uniformBuffers[i], uniformBuffersAllocations[i]);
	}
}

void Application::createDescriptorSetLayout() {
	vk::DescriptorSetLayoutBinding uboLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1,
	                                                vk::ShaderStageFlagBits::eVertex, nullptr};
	vk::DescriptorSetLayoutBinding samplerLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1,
	                                                    vk::ShaderStageFlagBits::eFragment, nullptr};
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
	vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, static_cast<uint32_t >(bindings.size()), bindings.data()};
	descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}


void Application::createIndexBuffer() {
	std::cout << "index count when creating buffer: " << indices.size() << std::endl;
	auto[buffer, allocation] =VulkanMemoryManager::getInstance()
			->createBufferTypeFromVector(indices, vk::BufferUsageFlagBits::eIndexBuffer);
	indexBuffer = buffer;
	indexBufferAllocation = allocation;
}

void Application::createVertexBuffer() {
	std::cout << "I have " << vertices.size() << " vertices" << std::endl;
	std::cout << "Total stride: " << sizeof(Vertex) << std::endl;
	auto[buffer, allocation] = VulkanMemoryManager::getInstance()
			->createBufferTypeFromVector(vertices, vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBuffer = buffer;
	vertexBufferAllocation = allocation;
}

void Application::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::ImageSubresourceLayers subresource{vk::ImageAspectFlagBits::eColor, 0, 0, 1};
	vk::Offset3D imageOffset{0, 0, 0};
	vk::Extent3D imageExtent{width, height, 1};
	vk::BufferImageCopy region(0, 0, 0, subresource, imageOffset, imageExtent);
	commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	endSingleTimeCommands(commandBuffer);
}

void Application::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                                        vk::ImageLayout newLayout, uint32_t inMipLevels) {
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::ImageSubresourceRange imageSubresourceRange({}, 0, inMipLevels, 0, 1);
	vk::ImageMemoryBarrier barrier({}, {}, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
	                               image, imageSubresourceRange);
	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;
	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	} else {
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
	           newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else if (oldLayout == vk::ImageLayout::eUndefined &&
	           newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.dstAccessMask =
				vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
	endSingleTimeCommands(commandBuffer);
}

void Application::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	vk::SemaphoreCreateInfo semaphoreInfo{};
	vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
		renderFinishedSemaphores[i] = device.createSemaphore(semaphoreInfo);
		inFlightFences[i] = device.createFence(fenceInfo);
	}
}

void Application::createCommandBuffers() {
	using namespace std;
	vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary,
	                                        static_cast<uint32_t>(swapChainFramebuffers.size()));
	commandBuffers.resize(swapChainFramebuffers.size());
	if (device.allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess) {
		throw runtime_error("failed to allocate command buffers!");
	}
	for (auto i{0}; i < swapChainFramebuffers.size(); i++) {
		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		commandBuffers[i].begin(beginInfo);
		vk::Rect2D renderArea{vk::Offset2D(0, 0), swapChainExtent};
		array<float, 4> clearColor{0.0f, 0.0f, 0.0f, 1.0f};
		vk::ClearDepthStencilValue depthStencilValue{1.0f, 0};
		array<vk::ClearValue, 2> clearValues{vk::ClearColorValue(clearColor), depthStencilValue};
		vk::RenderPassBeginInfo renderPassInfo(renderPass, swapChainFramebuffers[i], renderArea,
		                                       static_cast<uint32_t >(clearValues.size()), clearValues.data());
		commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
		vk::Buffer vertexBuffers[] = {vertexBuffer};
		vk::DeviceSize offsets[] = {0};
		commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
		commandBuffers[i].bindIndexBuffer(indexBuffer, offsets[0], vk::IndexType::eUint32);
		commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1,
		                                     descriptorSets.data(), 0,
		                                     nullptr);
		commandBuffers[i].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
	}
}

void Application::cleanupSwapChain() {
	cleanupImageResources();
	for (auto swapChainFramebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, swapChainFramebuffer, nullptr);
	}
	cleanupPipelineResources();
	for (auto swapChainImageView: swapChainImageViews) {
		vkDestroyImageView(device, swapChainImageView, nullptr);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VulkanMemoryManager::getInstance()->DestroyBuffer(uniformBuffers[i], uniformBuffersAllocations[i]);
	}
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void Application::cleanupPipelineResources() const {
	device.freeCommandBuffers(commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
}

void Application::cleanupImageResources() const {
	vkDestroyImageView(device, colorImageView, nullptr);
	VulkanMemoryManager::getInstance()->DestroyImage(colorImage, colorImageMemory);
	vkDestroyImageView(device, depthImageView, nullptr);
	VulkanMemoryManager::getInstance()->DestroyImage(depthImage, depthImageMemory);
}

void Application::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	vk::CommandPoolCreateInfo poolInfo({}, queueFamilyIndices.graphicsFamily.value());
	if (device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
		throw std::runtime_error(COMMAND_POOL_FAIL_CREATE_MSG);
	}
}

void Application::createFramebuffers() {
	swapChainFramebuffers.clear();
	std::transform(swapChainImageViews.begin(), swapChainImageViews.end(), std::back_inserter(swapChainFramebuffers),
	               [&](const vk::ImageView &view) {
		               std::array<vk::ImageView, 3> attachments{colorImageView, depthImageView, view};
		               vk::FramebufferCreateInfo framebufferInfo({}, renderPass,
		                                                         static_cast<uint32_t >(attachments.size()),
		                                                         attachments.data(),
		                                                         swapChainExtent.width, swapChainExtent.height, 1);
		               return device.createFramebuffer(framebufferInfo);
	               });
}

void Application::createRenderPass() {
	vk::AttachmentDescription colorAttachment({}, swapChainImageFormat, msaaSamples, vk::AttachmentLoadOp::eClear,
	                                          vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
	                                          vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
	                                          vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentDescription depthAttachment({}, findDepthFormat(), msaaSamples, vk::AttachmentLoadOp::eClear,
	                                          vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
	                                          vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
	                                          vk::ImageLayout::eDepthStencilAttachmentOptimal);
	vk::AttachmentDescription colorAttachmentResolve({}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
	                                                 vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
	                                                 vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
	                                                 vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	vk::AttachmentReference colorAttachmentResolveRef(2, vk::ImageLayout::eColorAttachmentOptimal);
	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentRef,
	                               &colorAttachmentResolveRef, &depthAttachmentRef);
	std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
	                                 vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
	                                 vk::AccessFlagBits::eColorAttachmentRead |
	                                 vk::AccessFlagBits::eColorAttachmentWrite);
	vk::RenderPassCreateInfo renderPassInfo({}, static_cast<uint32_t >(attachments.size()), attachments.data(), 1,
	                                        &subpass, 1, &dependency);
	if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
		throw std::runtime_error(RENDER_PASS_CREATE_FAIL_MSG);
	}

}

void Application::createImageViews() {
	swapChainImageViews.clear();
	std::transform(swapChainImages.begin(), swapChainImages.end(), std::back_inserter(swapChainImageViews),
	               [&](const vk::Image &image) -> vk::ImageView {
		               return createImageView(image, swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);
	               });
}

void Application::createSwapChain() {
	auto swapChainSupport = querySwapChainSupport(physicalDevice);
	auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	auto extent = chooseSwapExtent(swapChainSupport.capabilities);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	imageCount = std::clamp(imageCount,
	                        swapChainSupport.capabilities.minImageCount,
	                        (std::max)(imageCount, swapChainSupport.capabilities.maxImageCount));
	vk::SwapchainCreateInfoKHR createInfo({}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
	                                      extent, 1u, vk::ImageUsageFlagBits::eColorAttachment);
	QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
	if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2u;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	if (device.createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess) {
		throw std::runtime_error(SWAP_CHAIN_CREATE_FAIL_MSG);
	}
	swapChainImages = device.getSwapchainImagesKHR(swapChain);
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Application::createSurface() {
	using namespace std;
	//Using C api here since glfw expects C api surface
	VkSurfaceKHR temp{};
	if (glfwCreateWindowSurface(instance, window, nullptr, &temp) != VK_SUCCESS) {
		throw runtime_error(WINDOW_SURF_CREATE_FAIL_MSG);
	}
	surface = temp;
}

void Application::createLogicalDevice() {
	QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
	std::set<uint32_t> uniqueQueueFamilies{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
	float queuePriority = 1.0f;
	std::transform(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end(), std::back_inserter(queueCreateInfos),
	               [&](uint32_t family) -> vk::DeviceQueueCreateInfo { return {{}, family, 1, &queuePriority}; });
	vk::PhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
	vk::DeviceCreateInfo createInfo({}, static_cast<uint32_t>(queueCreateInfos.size()),
	                                queueCreateInfos.data(), static_cast<uint32_t >(validationLayers.size()),
	                                validationLayers.data(), static_cast<uint32_t >(requiredDeviceExtensions.size()),
	                                requiredDeviceExtensions.data(), &deviceFeatures);
	if (physicalDevice.createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess) {
		throw std::runtime_error(LOGI_DEV_CREATE_FAIL_MSG);
	}
	// 0 is the Queue index. Since creating only one, it is zero.
	device.getQueue(queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
	device.getQueue(queueIndices.presentFamily.value(), 0, &presentQueue);
}

size_t Application::scoreDevice(vk::PhysicalDevice deviceToScore) {
	vk::PhysicalDeviceProperties deviceProperties{};
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceToScore.getProperties(&deviceProperties);
	deviceToScore.getFeatures(&deviceFeatures);
	size_t
	score{};
	if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;
	score += deviceProperties.limits.maxImageDimension2D;
	// and so on...
	return score;
}

vk::Extent2D Application::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t >(height)};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
		                                capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
		                                 capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

vk::PresentModeKHR
Application::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
	using namespace std;
	vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;
	auto supports_mailbox = [&](const vk::PresentModeKHR &val) { return val == vk::PresentModeKHR::eMailbox; };
	auto supports_fifo_immediate = [&](const vk::PresentModeKHR &val) {
		return bestMode == vk::PresentModeKHR::eFifo && val == vk::PresentModeKHR::eImmediate;
	};
	auto best = find_if(availablePresentModes.begin(), availablePresentModes.end(), supports_mailbox);
	if (best == availablePresentModes.end()) {
		best = find_if(availablePresentModes.begin(), availablePresentModes.end(), supports_fifo_immediate);
	}
	bestMode = (best != availablePresentModes.end()) ? *best : bestMode;
	return *best;
}

vk::SurfaceFormatKHR
Application::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
	using namespace std;
	//surface has no preference...good
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
		return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT};
	}
	auto found = find_if(availableFormats.begin(), availableFormats.end(), [&](const vk::SurfaceFormatKHR &val) {
		return val.format == vk::Format::eB8G8R8A8Unorm &&
		       val.colorSpace == vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT;
	});
	if (found != availableFormats.end()) return *found;
	// if we don't get what we want, then just go with first entry
	return availableFormats[0];
}

bool Application::isDeviceSuitable(vk::PhysicalDevice deviceToTest) {
	vk::PhysicalDeviceProperties deviceProperties{};
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceToTest.getProperties(&deviceProperties);
	deviceToTest.getFeatures(&deviceFeatures);
	// Only caring about having a geometry shader right now
	QueueFamilyIndices queueIndices = findQueueFamilies(deviceToTest);
	bool extensionsSupported = checkDeviceExtensionSupport(deviceToTest);
	bool swapChainSuitable = false;
	if (extensionsSupported) {
		auto supportDetails = querySwapChainSupport(deviceToTest);
		swapChainSuitable = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
	}
	return static_cast<bool>(queueIndices.isComplete() && extensionsSupported && deviceFeatures.geometryShader &&
	                         swapChainSuitable && deviceFeatures.samplerAnisotropy);
}

bool Application::checkDeviceExtensionSupport(vk::PhysicalDevice deviceToCheck) {
	using namespace std;
	vector<vk::ExtensionProperties> availableExtensions{deviceToCheck.enumerateDeviceExtensionProperties()};
	set<string> requiredExtensions{requiredDeviceExtensions.begin(), requiredDeviceExtensions.end()};
	for (const auto &extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

void Application::pickPhysicalDevice() {
	using namespace std;
	vector<vk::PhysicalDevice> devices{instance.enumeratePhysicalDevices()};
	if (devices.empty()) throw runtime_error(NO_VULK_DEV_AVAILABLE_MSG);
	devices.erase(find_if(devices.begin(), devices.end(),
	                      [&](vk::PhysicalDevice &dev) { return !isDeviceSuitable(dev); }), devices.end());
	if (devices.empty()) throw runtime_error(VULK_DEV_NOT_SUITABLE_MSG);
	sort(devices.begin(), devices.end(), [&](vk::PhysicalDevice &a, vk::PhysicalDevice &b) {
		return scoreDevice(a) > scoreDevice(b);
	});
	physicalDevice = devices.front();
	msaaSamples = getMaxUsableSampleCount();
}

void Application::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) const {
	vk::DebugUtilsMessengerCreateInfoEXT info({}, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
	                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
	                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
	                                          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
	                                          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
	                                          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance, debugCallback, {});
	createInfo = info;
}

Application::SwapChainSupportDetails
Application::querySwapChainSupport(vk::PhysicalDevice deviceToQuery) {
	SwapChainSupportDetails details{deviceToQuery.getSurfaceCapabilitiesKHR(surface),
	                                deviceToQuery.getSurfaceFormatsKHR(surface),
	                                deviceToQuery.getSurfacePresentModesKHR(surface)};
	return details;
}

Application::QueueFamilyIndices Application::findQueueFamilies(vk::PhysicalDevice deviceToSearch) {
	QueueFamilyIndices queueIndices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(deviceToSearch, &queueFamilyCount, nullptr);
	std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
	deviceToSearch.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());
	uint32_t i = 0;
	vk::Bool32 presentSupport{VK_FALSE};
	for (const auto &queueFamily : queueFamilies) {
		vkGetPhysicalDeviceSurfaceSupportKHR(deviceToSearch, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0) {
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				queueIndices.graphicsFamily = i;
			}
			if (presentSupport) {
				queueIndices.presentFamily = i;
			}
		}
		if (queueIndices.isComplete()) {
			break;
		}
		i++;
	}
	return queueIndices;
}

void Application::recreateSwapChain() {
	int width{}, height{};
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device);
	cleanupSwapChain();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void Application::initVulkanBeforePipeline() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	initGlobalVmaAllocator();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
}

void Application::initVulkanAfterPipeline() {
	createCommandPool();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	readModelFile("../resources/models/chalet.obj");
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

void Application::createGraphicsPipelineFromDescriptions(vk::VertexInputBindingDescription &bindingDescription,
                                                         std::array<vk::VertexInputAttributeDescription, 3> &attributeDescriptions) {
	auto vert{readFile("../shaders/sprvs/depth_vert.spv")};
	auto frag{readFile("../shaders/sprvs/textured_frag.spv")};
	auto verMod{createShaderModule(vert)};
	auto fraMod{createShaderModule(frag)};
	bindingDescription = Vertex::getBindingDescription();
	attributeDescriptions = Vertex::getAttributeDescriptions();
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex, verMod, "main", {});
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment, fraMod, "main", {});
	vk::PipelineShaderStageCreateInfo stages[] = {vertShaderStageInfo, fragShaderStageInfo};
	vk::VertexInputBindingDescription &bindingDescription1 = bindingDescription;
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, 1, &bindingDescription1,
	                                                       static_cast<uint32_t >( attributeDescriptions.size()),
	                                                       attributeDescriptions.data());
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	vk::Viewport viewport(0.0f, 0.0f, static_cast<float >( swapChainExtent.width),
	                      static_cast<float >( swapChainExtent.height), 0.0f, 1.0f);
	vk::Rect2D scissor(vk::Offset2D{0, 0}, swapChainExtent);
	vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);
	vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
	                                                    vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
	                                                    VK_FALSE, {}, {}, {}, 1.0f);
	vk::PipelineMultisampleStateCreateInfo multisampling({}, msaaSamples, VK_TRUE, 0.2f, {}, VK_FALSE, VK_FALSE);
	vk::PipelineColorBlendAttachmentState colorBlendAttachment(VK_FALSE, vk::BlendFactor::eOne, vk::BlendFactor::eZero,
	                                                           vk::BlendOp::eAdd, vk::BlendFactor::eOne,
	                                                           vk::BlendFactor::eZero, vk::BlendOp::eAdd,
	                                                           {vk::ColorComponentFlagBits::eR |
	                                                            vk::ColorComponentFlagBits::eG |
	                                                            vk::ColorComponentFlagBits::eB |
	                                                            vk::ColorComponentFlagBits::eA});
	const std::array<float, 4> blendConstants{0.0f, 0.0f, 0.0f, 0.0f};
	vk::PipelineColorBlendStateCreateInfo colorBlending({}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment,
	                                                    blendConstants);
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 1, &descriptorSetLayout, 0, {});
	if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
		throw std::runtime_error(PIPELINE_LAYOUT_CREATE_FAIL_MSG);
	}
	vk::PipelineDepthStencilStateCreateInfo depthStencil({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE,
	                                                     {}, {}, 0.0f, 1.0f);
	vk::PipelineVertexInputStateCreateInfo &vertexInputInfo1 = vertexInputInfo;
	vk::PipelineInputAssemblyStateCreateInfo &inputAssembly1 = inputAssembly;
	vk::PipelineViewportStateCreateInfo &viewportState1 = viewportState;
	vk::PipelineRasterizationStateCreateInfo &rasterizer1 = rasterizer;
	vk::PipelineMultisampleStateCreateInfo &multisampling1 = multisampling;
	vk::PipelineColorBlendStateCreateInfo &colorBlending1 = colorBlending;
	vk::PipelineDepthStencilStateCreateInfo &depthStencil1 = depthStencil;
	const vk::GraphicsPipelineCreateInfo pipelineInfo1({}, 2, stages, &vertexInputInfo1, &inputAssembly1, {},
	                                                   &viewportState1, &rasterizer1, &multisampling1, &depthStencil1,
	                                                   &colorBlending1, {}, pipelineLayout, renderPass, {}, {}, {});
	const vk::GraphicsPipelineCreateInfo &pipelineInfo = pipelineInfo1;
	graphicsPipeline = device.createGraphicsPipeline({}, pipelineInfo);
	vkDestroyShaderModule(device, verMod, nullptr);
	vkDestroyShaderModule(device, fraMod, nullptr);
}

void Application::processSceneObject(const aiScene *scene) {
	using namespace std;
	cout << "There are: " << scene->mNumMeshes << " meshes stored." << endl;
	auto root = scene->mRootNode;
	cout << "Root has " << root->mNumChildren << " children." << endl;
	cout << "Root also has: " << root->mNumMeshes << " meshes associated with it." << endl;
	aiMesh *mesh{scene->mMeshes[0]};
	if (root->mNumChildren) {
		cout << "Child of root:" << endl;
		auto child = root->mChildren[0];
		cout << "has " << root->mChildren[0]->mNumChildren << " amount of children" << endl;
		cout << "Child has: " << child->mNumMeshes << " meshes associated with it." << endl;
		mesh = scene->mMeshes[child->mMeshes[0]];
	}
	cout << "There are: " << scene->mNumMaterials << " materials" << endl;
	if (mesh->HasTextureCoords(0)) {
		cout << "The given mesh has texture coordinates" << endl;
		cout << "There are " << mesh->GetNumUVChannels() << " UV channels" << endl;
	}
	if (mesh->HasVertexColors(0)) {
		cout << "The given mesh has vertex colors" << endl;
	}
	cout << "Going to read in: " << mesh->mNumVertices << " amount of vertices." << endl;
	vertices.resize(mesh->mNumVertices);
	indices.clear();
	int channel = 0;
	cout << "Using channel " << channel << " for getting UV coord" << endl;
	for (size_t i{}; i < mesh->mNumVertices; i++) {
		auto vertex_point = mesh->mVertices[i];
		auto color_point = mesh->mTextureCoords[channel][i];
		Vertex toAdd{{vertex_point[0], vertex_point[1], vertex_point[2]},
		             {1.0f,            1.0f,            1.0f},
		             {color_point[0],
		                               color_point[1]}};

		vertices[i] = toAdd;

	}
	cout << "There are now " << vertices.size() << " vertices read in" << endl;
	cout << "There are " << mesh->mNumFaces << " number of faces." << endl;
	for (size_t i{}; i < mesh->mNumFaces; i++) {
		for (size_t j{}; j < mesh->mFaces[i].mNumIndices; j++) {
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}
	cout << "There are now: " << indices.size() << " number of indices" << endl;
}

void Application::generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight,
                                  uint32_t inMipLevels) {
	// Check if image format supports linear blitting
	vk::FormatProperties formatProperties;
	physicalDevice.getFormatProperties(imageFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
		throw std::runtime_error(TEXTURE_FORMAT_NOT_SUPPORT_BLITTING_MSG);
	}
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::ImageMemoryBarrier barrier({}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image,
	                               {vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1});
	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;
	for (uint32_t i = 1; i < inMipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
		                              0, nullptr, 0, nullptr, 1, &barrier);
		const std::array<vk::Offset3D, 2> srcOffset{vk::Offset3D{0, 0, 0},
		                                            vk::Offset3D{mipWidth, mipHeight, 1}};
		const std::array<vk::Offset3D, 2> dstOffset{vk::Offset3D{0, 0, 0},
		                                            vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1,
		                                                         mipHeight > 1 ? mipHeight / 2 : 1, 1}};
		vk::ImageBlit blit({vk::ImageAspectFlagBits::eColor, i - 1, 0, 1}, srcOffset,
		                   {vk::ImageAspectFlagBits::eColor, i, 0, 1}, dstOffset);
		commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal,
		                        image, vk::ImageLayout::eTransferDstOptimal,
		                        1, &blit, vk::Filter::eLinear);
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
		                              {}, 0, nullptr, 0, nullptr, 1, &barrier);
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}
	barrier.subresourceRange.baseMipLevel = inMipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
	                              0, nullptr, 0, nullptr, 1, &barrier);
	endSingleTimeCommands(commandBuffer);
}

vk::SampleCountFlagBits Application::getMaxUsableSampleCount() {
	vk::PhysicalDeviceProperties physicalDeviceProperties;
	physicalDevice.getProperties(&physicalDeviceProperties);
	vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
	                              physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
	if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
	if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
	if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
	if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
	if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }
	return vk::SampleCountFlagBits::e1;
}

void Application::createColorResources() {
	vk::Format colorFormat = swapChainImageFormat;
	createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, vk::ImageTiling::eOptimal,
	            vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
	            VMA_MEMORY_USAGE_GPU_ONLY, colorImage, colorImageMemory);
	colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
}

Application::Application()
		: vertices{
		{{-0.5f, -0.5f, 0.0f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f,  -0.5f, 0.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f,  0.5f,  0.0f},  {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f,  0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		{{-1.5f, -1.5f, -1.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{1.5f,  -1.5f, -1.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{1.5f,  1.5f,  -1.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-1.5f, 1.5f,  -1.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}},
		  indices{
				  0, 1, 2, 2, 3, 0,
				  4, 5, 6, 6, 7, 4} {
}

void Application::initGlobalVmaAllocator() {
	VulkanMemoryManager::Init(device, physicalDevice);
}

void Application::destroyGlobalAllocator() {
	VulkanMemoryManager::Destroy();
}


