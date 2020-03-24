//
// Created by alexa on 5/18/2019.
//

#include "TriangleApplication.h"

bool TriangleApplication::readModelFile(const std::string &pFile) {
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

vk::Result CreateDebugUtilsMessengerEXT(vk::Instance instance, const vk::DebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const vk::AllocationCallbacks *pAllocator,
                                        vk::DebugUtilsMessengerEXT *pDebugMessenger) {
	return instance.createDebugUtilsMessengerEXT(pCreateInfo, pAllocator, pDebugMessenger);
}

void DestroyDebugUtilsMessengerEXT(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger,
                                   const vk::AllocationCallbacks *pAllocator) {
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, pAllocator);
}

void TriangleApplication::setupDebugMessenger() {
	//If no validation requested, just do nothing
	vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void TriangleApplication::cleanup() {
	cleanupSwapChain();
	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);
	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	// Can add logic to test if we are debugging or not
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void TriangleApplication::drawFrame() {
	using namespace std;
	device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	uint32_t imageIndex;
	auto result = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(),
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

void TriangleApplication::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(device);
}

void TriangleApplication::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetWindowUserPointer(window, this);
}

vk::ShaderModule TriangleApplication::createShaderModule(const std::vector<char> &code) {
	using namespace std;
	vk::ShaderModuleCreateInfo shaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t *>(code.data()));
	vk::ShaderModule module;
	if (device.createShaderModule(&shaderModuleCreateInfo, nullptr, &module) != vk::Result::eSuccess) {
		throw runtime_error("Uh oh spaghettio, no shader module created.");
	}
	return module;
}

std::vector<char> TriangleApplication::readFile(const std::string &filename) {
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

void TriangleApplication::framebufferResizeCallback(GLFWwindow *window, int, int) {
	auto app = reinterpret_cast<TriangleApplication * >(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void TriangleApplication::updateUniformBuffer(uint32_t currentImage) {
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
	vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

vk::Bool32 TriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

void TriangleApplication::validateLayerSupport() {
	using namespace std;
	uint32_t layerCount{};
	vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
	vector<vk::LayerProperties> availableLayers(layerCount);
	vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (auto layerName: validationLayers) {
		string toFind{layerName};
		auto found = find_if(availableLayers.begin(), availableLayers.end(), [&](const vk::LayerProperties &props) {
			string currName{props.layerName};
			return currName == toFind;
		});
		if (found == availableLayers.end()) {
			throw runtime_error("Requested validation layer does not exist.");
		}
	}
}

std::vector<const char *> TriangleApplication::getRequiredExtensions() const {
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

void TriangleApplication::validateExtensions(const std::vector<const char *> &toValidate) const {
	using namespace std;
	// Get all supported extensions
	vector<vk::ExtensionProperties> extensions{};
	vk::enumerateInstanceExtensionProperties({""}, extensions.get_allocator(), {});
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

vk::InstanceCreateInfo TriangleApplication::createInstanceCreateInfo(vk::ApplicationInfo &appInfo) {
	vk::InstanceCreateInfo info({}, &appInfo, validationLayers.size(), validationLayers.data(), {}, {});
	return info;
}

vk::ApplicationInfo TriangleApplication::createApplicationInfo() const {
	vk::ApplicationInfo info("Hello World", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0),
	                         VK_API_VERSION_1_0);
	return info;
}

void TriangleApplication::createInstance() {
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

void TriangleApplication::createDepthResources() {
	vk::Format depthFormat = findDepthFormat();
	createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, vk::ImageTiling::eOptimal,
	            vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage,
	            depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
	transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
}

vk::Format TriangleApplication::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
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

vk::Format TriangleApplication::findDepthFormat() {
	return findSupportedFormat(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			vk::ImageTiling::eOptimal,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool TriangleApplication::hasStencilComponent(vk::Format format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void TriangleApplication::createTextureSampler() {
	vk::SamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f; // Optional
	samplerInfo.maxLod = static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0.0f; // Optional
	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void TriangleApplication::createTextureImageView() {
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, vk::ImageAspectFlagBits::eColor,
	                                   mipLevels);
}

vk::ImageView TriangleApplication::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags,
                                                   uint32_t inMipLevels) {
	vk::ImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = inMipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	vk::ImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void TriangleApplication::createTextureImage() {
	int texWidth, texHeight, texChannels;
	stbi_uc *pixels = stbi_load("../resources/textures/chalet.jpg", &texWidth, &texHeight,
	                            &texChannels,
	                            STBI_rgb_alpha);
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
	auto imageSize = static_cast<vk::DeviceSize>(static_cast<vk::DeviceSize>(texWidth) * texHeight * 4);
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
	             stagingBufferMemory);
	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);
	stbi_image_free(pixels);
	createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), mipLevels,
	            vk::SampleCountFlagBits::e1,
	            VK_FORMAT_R8G8B8A8_UNORM,
	            vk::ImageTiling::eOptimal,
	            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
	                      vk::ImageLayout::eTransferDstOptimal, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void TriangleApplication::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	vk::SubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

vk::CommandBuffer TriangleApplication::beginSingleTimeCommands() {
	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void
TriangleApplication::createImage(uint32_t width, uint32_t height, uint32_t imageMipLevels,
                                 vk::SampleCountFlagBits numSamples,
                                 vk::Format format,
                                 vk::ImageTiling tiling,
                                 vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image,
                                 vk::DeviceMemory &imageMemory) {
	vk::ImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = imageMipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create image!");
	}

	vk::MemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, imageMemory, 0);

}

void TriangleApplication::createDescriptorSets() {
	std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();
	descriptorSets.resize(swapChainImages.size());
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vk::DescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;
		std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
		                       nullptr);
	}
}

void TriangleApplication::createDescriptorPool() {
	vk::DescriptorPoolSize uboSize = {};
	uboSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	vk::DescriptorPoolSize samplerSize = {};
	samplerSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	std::vector<vk::DescriptorPoolSize> poolSizes{uboSize, samplerSize};
	vk::DescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void TriangleApplication::createUniformBuffers() {
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
		             uniformBuffersMemory[i]);
	}

}

void TriangleApplication::createDescriptorSetLayout() {
	vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
	vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
	vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t  > (bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void TriangleApplication::createIndexBuffer() {
	std::cout << "index count when creating buffer: " << indices.size() << std::endl;
//	std::copy(indices.begin(), indices.end(), std::ostream_iterator<uint32_t>(std::cout, ", "));
	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
	             stagingBufferMemory);

	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void TriangleApplication::createVertexBuffer() {
	std::cout << "I have " << vertices.size() << " vertices" << std::endl;
//	std::copy(vertices.begin(), vertices.end(), std::ostream_iterator<Vertex>(std::cout));
	std::cout << "Total stride: " << sizeof(Vertex) << std::endl;
	vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
	             stagingBufferMemory);

	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void TriangleApplication::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
			width,
			height,
			1
	};
	vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			1,
			&region
	);

	endSingleTimeCommands(commandBuffer);
}

void TriangleApplication::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                                                vk::ImageLayout newLayout, uint32_t inMipLevels) {
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::ImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = inMipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO
	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
	           newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
	           newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}


	vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
	);

	endSingleTimeCommands(commandBuffer);

}

void TriangleApplication::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

void
TriangleApplication::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                                  vk::Buffer &buffer, vk::DeviceMemory &bufferMemory) {
	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create buffer!");
	}

	vk::MemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	vkBindBufferMemory(device, buffer, bufferMemory, 0);

}

void TriangleApplication::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	vk::SemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vk::FenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
		    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
		    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

}

void TriangleApplication::createCommandBuffers() {
	commandBuffers.resize(swapChainFramebuffers.size());
	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional
		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		vk::RenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapChainExtent;
		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = {1.0f, 0}; // reset to furthest value away
		renderPassInfo.clearValueCount = static_cast<uint32_t > (clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vk::Buffer vertexBuffers[] = {vertexBuffer};
		vk::DeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		                        &descriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);
		if (vkEndCommandBuffer(commandBuffers[i]) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to record command buffer!");
		}


	}


}

void TriangleApplication::cleanupSwapChain() {
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
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void TriangleApplication::cleanupPipelineResources() const {
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()),
	                     commandBuffers.data());
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
}

void TriangleApplication::cleanupImageResources() const {
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
}

void TriangleApplication::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	vk::CommandPoolCreateInfo poolInfo{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			0, // Optional flags
			queueFamilyIndices.graphicsFamily.value()
	};
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
		throw std::runtime_error(COMMAND_POOL_FAIL_CREATE_MSG);
	}
}

void TriangleApplication::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (unsigned long i{0}; i < swapChainImageViews.size(); i++) {
		std::array<vk::ImageView, 3> attachments[] = {
				{colorImageView,
						depthImageView,
						swapChainImageViews[i]}

		};
		vk::FramebufferCreateInfo framebufferInfo{
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				nullptr,
				0, //Optional flags
				renderPass,
				static_cast<uint32_t >(attachments->size()),
				attachments->data(),
				swapChainExtent.width,
				swapChainExtent.height,
				1 //Layer count
		};
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
			throw std::runtime_error(FRAMEBUFFER_CREATE_FAIL_MSG);
		}
	}

}

void TriangleApplication::createRenderPass() {
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	vk::AttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	vk::AttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;
	std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
		throw std::runtime_error(RENDER_PASS_CREATE_FAIL_MSG);
	}

}

void TriangleApplication::createImageViews() {
	swapChainImageViews.clear();
	std::for_each(swapChainImages.begin(), swapChainImages.end(),
	              [&](const vk::Image &image) {
		              swapChainImageViews.push_back(
				              createImageView(image, swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1));
	              });
}

void TriangleApplication::createSwapChain() {
	auto swapChainSupport = querySwapChainSupport(physicalDevice);
	auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	auto extent = chooseSwapExtent(swapChainSupport.capabilities);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	imageCount = std::clamp(imageCount,
	                        swapChainSupport.capabilities.minImageCount,
	                        std::max(imageCount, swapChainSupport.capabilities.maxImageCount));
	vk::SwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1u;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
	if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2u;
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
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != vk::Result::eSuccess) {
		throw std::runtime_error(SWAP_CHAIN_CREATE_FAIL_MSG);
	}
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void TriangleApplication::createSurface() {
	using namespace std;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != vk::Result::eSuccess) {
		throw runtime_error(WINDOW_SURF_CREATE_FAIL_MSG);
	}
}

void TriangleApplication::createLogicalDevice() {
	QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
	std::set<uint32_t> uniqueQueueFamilies{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
	float queuePriority = 1.0f;
	for (auto &family : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueFamilyIndex = family;
		info.queueCount = 1;
		info.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(info);
	}
	vk::DeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t >(queueCreateInfos.size());
	vk::PhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t >(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
	// Can add check here to see if validation is enabled. Not really used, but good for legacy support
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != vk::Result::eSuccess) {
		throw std::runtime_error(LOGI_DEV_CREATE_FAIL_MSG);
	}
	// 0 is the Queue index. Since creating only one, it is zero.
	vkGetDeviceQueue(device, queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueIndices.presentFamily.value(), 0, &presentQueue);

}

size_t TriangleApplication::scoreDevice(vk::PhysicalDevice deviceToScore) {
	vk::PhysicalDeviceProperties deviceProperties{};
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceToScore.getProperties(&deviceProperties);
	deviceToScore.getFeatures(&deviceFeatures);
	size_t score{};
	if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;
	score += deviceProperties.limits.maxImageDimension2D;
	// and so on...
	return score;
}

vk::Extent2D TriangleApplication::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
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
TriangleApplication::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
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
TriangleApplication::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
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

bool TriangleApplication::isDeviceSuitable(vk::PhysicalDevice deviceToTest) {
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

bool TriangleApplication::checkDeviceExtensionSupport(vk::PhysicalDevice deviceToCheck) {
	using namespace std;
	vector<vk::ExtensionProperties> availableExtensions{};
	deviceToCheck.enumerateDeviceExtensionProperties(nullptr, availableExtensions.get_allocator());
	set<string> requiredExtensions{requiredDeviceExtensions.begin(), requiredDeviceExtensions.end()};
	for (const auto &extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

void TriangleApplication::pickPhysicalDevice() {
	using namespace std;
	vector<vk::PhysicalDevice> devices{};
	instance.enumeratePhysicalDevices(devices.get_allocator());
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

void TriangleApplication::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) const {
	vk::DebugUtilsMessengerCreateInfoEXT info({}, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
	                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
	                                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
	                                          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
	                                          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
	                                          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance, debugCallback, {});

	createInfo = info;
}

TriangleApplication::SwapChainSupportDetails
TriangleApplication::querySwapChainSupport(vk::PhysicalDevice deviceToQuery) {
	SwapChainSupportDetails details;
	deviceToQuery.getSurfaceCapabilitiesKHR(surface, &details.capabilities);
	deviceToQuery.getSurfaceFormatsKHR(surface, details.formats.get_allocator());
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToQuery, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		deviceToQuery.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

uint32_t TriangleApplication::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
	vk::PhysicalDeviceMemoryProperties memProperties;
	physicalDevice.getMemoryProperties(&memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error(NO_SUITABLE_MEMORY_MSG);
}

TriangleApplication::QueueFamilyIndices TriangleApplication::findQueueFamilies(vk::PhysicalDevice deviceToSearch) {
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

void TriangleApplication::recreateSwapChain() {
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

void TriangleApplication::initVulkanBeforePipeline() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
}

void TriangleApplication::initVulkanAfterPipeline() {
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

void TriangleApplication::createGraphicsPipelineFromDescriptions(vk::VertexInputBindingDescription &bindingDescription,
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
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, 1, &bindingDescription1, attributeDescriptions.size(),
	                                                       attributeDescriptions.data());
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	vk::Viewport viewport(0.0f, 0.0f, swapChainExtent.width, swapChainExtent.height, 0.0f, 1.0f);
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

void TriangleApplication::processSceneObject(const aiScene *scene) {
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

void TriangleApplication::generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight,
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

vk::SampleCountFlagBits TriangleApplication::getMaxUsableSampleCount() {
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

void TriangleApplication::createColorResources() {

	vk::Format colorFormat = swapChainImageFormat;

	createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, vk::ImageTiling::eOptimal,
	            vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
	            vk::MemoryPropertyFlagBits::eDeviceLocal, colorImage, colorImageMemory);
	colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
}

TriangleApplication::TriangleApplication()
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
