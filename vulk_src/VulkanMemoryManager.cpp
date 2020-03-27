//
// Created by alexa on 3/25/2020.
//

#include "VulkanMemoryManager.h"
void VulkanMemoryManager::DestroyImageView(vk::ImageView view)
{
	logicalDevice.destroyImageView(view);
}

void VulkanMemoryManager::DestroySampler(vk::Sampler sampler)
{
	logicalDevice.destroySampler(sampler);
}

std::tuple<vk::Buffer, VmaAllocation> VulkanMemoryManager::initializeStagingBuffer(void *data, size_t dataSize)
{
	vk::Buffer stagingBuffer;
	VmaAllocation stagingBufferAllocation;
	createBuffer(dataSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer,
				 stagingBufferAllocation);
	void *temp_data{};
	vmaMapMemory(allocator, stagingBufferAllocation, &temp_data);
	memcpy(temp_data, data, dataSize);
	vmaUnmapMemory(allocator, stagingBufferAllocation);
	return std::make_tuple(stagingBuffer, stagingBufferAllocation);
}

void VulkanMemoryManager::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,
									   vk::Buffer &buffer, VmaAllocation &bufferAllocation)
{
	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	auto vkCBufferInfo = static_cast<VkBufferCreateInfo>(bufferInfo);
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = memoryUsage;
	VkBuffer vkCBuffer;
	vmaCreateBuffer(allocator, &vkCBufferInfo, &allocationCreateInfo, &vkCBuffer, &bufferAllocation, nullptr);
	buffer = vkCBuffer;
}

VulkanMemoryManager *VulkanMemoryManager::getInstance()
{
	return vmmInstance;
}

void VulkanMemoryManager::Destroy()
{
	vmaDestroyAllocator(vmmInstance->allocator);
	delete vmmInstance;
}

void VulkanMemoryManager::DestroyImage(vk::Image img, VmaAllocation imgMemory)
{
	vmaDestroyImage(allocator, img, imgMemory);
}

void VulkanMemoryManager::DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation)
{
	vmaDestroyBuffer(allocator, buff, buffAllocation);
}

std::tuple<vk::Image, VmaAllocation>
VulkanMemoryManager::createImage(VkImageCreateInfo imageInfo, VmaAllocationCreateInfo allocationCreateInfo)
{
	VkImage temp;
	VmaAllocation alloc;
	vmaCreateImage(allocator, &imageInfo, &allocationCreateInfo, &temp, &alloc, nullptr);
	return std::tuple<vk::Image, VmaAllocation>(temp, alloc);
}

void VulkanMemoryManager::CopyDataToAllocation(void *toCopy, vk::DeviceSize copySize, VmaAllocation allocation)
{
	void *data;
	vmaMapMemory(allocator, allocation, &data);
	memcpy(data, toCopy, copySize);
	vmaUnmapMemory(allocator, allocation);
}

void VulkanMemoryManager::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, uint64_t size)
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::BufferCopy copyRegion({}, {}, size);
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
	endSingleTimeCommands(commandBuffer);
}

void VulkanMemoryManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();
	vk::SubmitInfo submitInfo{{}, {}, {}, 1, &commandBuffer};
	graphicsQueue.submit(1, &submitInfo, {});
	graphicsQueue.waitIdle();
	logicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

vk::CommandBuffer VulkanMemoryManager::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo allocInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};
	auto buffers{logicalDevice.allocateCommandBuffers(allocInfo)};
	vk::CommandBuffer commandBuffer{buffers[0]};
	vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
	commandBuffer.begin(beginInfo);
	return commandBuffer;
}

vk::DeviceSize VulkanMemoryManager::GetAllocationSize(VmaAllocation allocation)
{
	VmaAllocationInfo toCheck;
	vmaGetAllocationInfo(allocator, allocation, &toCheck);
	return toCheck.size;
}

VulkanMemoryManager::VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physDevice, vk::CommandPool pool,
										 vk::Queue queue)
	: logicalDevice{device}, physicalDevice{physDevice},
	  commandPool{pool}, graphicsQueue{queue}
{
	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.physicalDevice = physDevice;
	allocatorCreateInfo.device = device;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

void VulkanMemoryManager::Init(vk::Device device, vk::PhysicalDevice physDevice, vk::CommandPool pool, vk::Queue queue)
{
	if (!vmmInstance)
	{
		vmmInstance = new VulkanMemoryManager(device, physDevice, pool, queue);
	}
}

std::tuple<vk::Image, VmaAllocation> VulkanMemoryManager::CreateImageFromData(void *data, vk::DeviceSize size, vk::ImageCreateInfo info, VmaMemoryUsage usage)
{
	auto [stagingBuffer, stagingBufferMemory] = initializeStagingBuffer(data, size);
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = usage;
	auto [retImg, imgAlloc] = createImage(info, allocationCreateInfo);
	transitionImageLayout(retImg, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined,
						  vk::ImageLayout::eTransferDstOptimal, info.mipLevels);
	copyBufferToImage(stagingBuffer, retImg, info);
	generateMipmaps(retImg, info);
	DestroyBuffer(stagingBuffer, stagingBufferMemory);
	return std::make_tuple(retImg, imgAlloc);
}

void VulkanMemoryManager::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
												vk::ImageLayout newLayout, uint32_t inMipLevels)
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::ImageSubresourceRange imageSubresourceRange({}, 0, inMipLevels, 0, 1);
	vk::ImageMemoryBarrier barrier({}, {}, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
								   image, imageSubresourceRange);
	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;
	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
			 newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined &&
			 newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.dstAccessMask =
			vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
	endSingleTimeCommands(commandBuffer);
}

bool VulkanMemoryManager::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void VulkanMemoryManager::copyBufferToImage(vk::Buffer buffer, vk::Image image, vk::ImageCreateInfo info)
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::ImageSubresourceLayers subresource{vk::ImageAspectFlagBits::eColor, 0, 0, 1};
	vk::Offset3D imageOffset{0, 0, 0};
	vk::BufferImageCopy region(0, 0, 0, subresource, imageOffset, info.extent);
	commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	endSingleTimeCommands(commandBuffer);
}

void VulkanMemoryManager::generateMipmaps(vk::Image image, vk::ImageCreateInfo info)
{
	// Check if image format supports linear blitting
	vk::FormatProperties formatProperties;
	physicalDevice.getFormatProperties(info.format, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error(TEXTURE_FORMAT_NOT_SUPPORT_BLITTING_MSG);
	}
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::ImageMemoryBarrier barrier({}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image,
								   {vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1});
	int32_t mipWidth = info.extent.width;
	int32_t mipHeight = info.extent.height;
	for (uint32_t i = 1; i < info.mipLevels; i++)
	{
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
		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}
	barrier.subresourceRange.baseMipLevel = info.mipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
								  0, nullptr, 0, nullptr, 1, &barrier);
	endSingleTimeCommands(commandBuffer);
}

vk::ImageView VulkanMemoryManager::CreateImageView(vk::ImageViewCreateInfo info)
{
	return logicalDevice.createImageView(info);
}
vk::Sampler VulkanMemoryManager::CreateImageSampler(vk::SamplerCreateInfo info)
{
	return logicalDevice.createSampler(info);
}
