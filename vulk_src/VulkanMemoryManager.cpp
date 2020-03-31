//
// Created by alexa on 3/25/2020.
//

#include "VulkanMemoryManager.h"

std::tuple<vk::Buffer, VmaAllocation> VulkanMemoryManager::initializeStagingBuffer(void *data, size_t dataSize)
{
	vk::Buffer stagingBuffer;
	VmaAllocation stagingBufferAllocation;
	createBuffer(dataSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer,
				 stagingBufferAllocation);
	void *ptrGpuMemory{};
	vmaMapMemory(allocator, stagingBufferAllocation, &ptrGpuMemory);
	memcpy(ptrGpuMemory, data, dataSize);
	vmaUnmapMemory(allocator, stagingBufferAllocation);
	return std::make_tuple(stagingBuffer, stagingBufferAllocation);
}

std::tuple<vk::Buffer, VmaAllocation> VulkanMemoryManager::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	auto vkCBufferInfo = static_cast<VkBufferCreateInfo>(bufferInfo);
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = memoryUsage;
	VkBuffer vkCBuffer;
	VmaAllocation bufferAllocation;
	vmaCreateBuffer(allocator, &vkCBufferInfo, &allocationCreateInfo, &vkCBuffer, &bufferAllocation, nullptr);
	buffer = vkCBuffer;
	return std::make_tuple<vk::Buffer, VmaAllocation>(buffer, bufferAllocation);
}

std::tuple<vk::Buffer, VmaAllocation> VulkanMemoryManager::StageData(void *data, size_t dataSize)
{
	auto [buffer, allocation] = createBuffer(dataSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
	void *ptrGpuMemory{};
	vmaMapMemory(allocator, stagingBufferAllocation, &ptrGpuMemory);
	memcpy(ptrGpuMemory, data, dataSize);
	vmaUnmapMemory(allocator, stagingBufferAllocation);
	return std::make_tuple(stagingBuffer, stagingBufferAllocation);
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

void VulkanMemoryManager::DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation)
{
	vmaDestroyBuffer(allocator, buff, buffAllocation);
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

VulkanMemoryManager::VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physDevice, vk::CommandPool pool, vk::Queue queue)
	: logicalDevice{device}, physicalDevice{physDevice}, commandPool{pool}, graphicsQueue{queue}
{
	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.physicalDevice = physDevice;
	allocatorCreateInfo.device = device;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

VmaAllocator GetAllocator()
{
	return allocator;
}

void VulkanMemoryManager::Init(vk::Device device, vk::PhysicalDevice physDevice, vk::CommandPool pool, vk::Queue queue)
{
	if (!vmmInstance)
	{
		vmmInstance = new VulkanMemoryManager(device, physDevice, pool, queue);
	}
}
