//
// Created by alexa on 3/25/2020.
//

#include "VulkanMemoryManager.h"

std::tuple<vk::Buffer, VmaAllocation> VulkanMemoryManager::initializeStagingBuffer(void *data, size_t dataSize) {
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

void
VulkanMemoryManager::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                                  vk::Buffer &buffer, VmaAllocation &bufferAllocation) {
	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	auto vkCBufferInfo = static_cast<VkBufferCreateInfo>(bufferInfo);
	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = memoryUsage;
	VkBuffer vkCBuffer;
	vmaCreateBuffer(allocator, &vkCBufferInfo, &allocationCreateInfo, &vkCBuffer, &bufferAllocation, nullptr);
	buffer = vkCBuffer;
}

void VulkanMemoryManager::Init(vk::Device device, vk::PhysicalDevice physicalDevice) {
	if (!vmmInstance) {
		vmmInstance = new VulkanMemoryManager(device, physicalDevice);
	}
}

VulkanMemoryManager::VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physicalDevice) {
	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

VulkanMemoryManager *VulkanMemoryManager::getInstance() {
	return vmmInstance;
}

void VulkanMemoryManager::Destroy() {
	vmaDestroyAllocator(vmmInstance->allocator);
	delete vmmInstance;
}

void VulkanMemoryManager::DestroyImage(vk::Image img, VmaAllocation imgMemory) {
	vmaDestroyImage(allocator, img, imgMemory);
}

void VulkanMemoryManager::DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation) {
	vmaDestroyBuffer(allocator, buff, buffAllocation);
}

std::tuple<vk::Image, VmaAllocation>
VulkanMemoryManager::createImage(VkImageCreateInfo imageInfo, VmaAllocationCreateInfo allocationCreateInfo) {
	VkImage temp;
	VmaAllocation alloc;
	vmaCreateImage(allocator, &imageInfo, &allocationCreateInfo, &temp, &alloc, nullptr);
	return std::tuple<vk::Image, VmaAllocation>(temp, alloc);
}

void VulkanMemoryManager::CopyDataToAllocation(void *toCopy, vk::DeviceSize copySize, VmaAllocation allocation) {
	void *data;
	vmaMapMemory(allocator, allocation, &data);
	memcpy(data, toCopy, copySize);
	vmaUnmapMemory(allocator, allocation);
}

void VulkanMemoryManager::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, uint64_t size) {
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
	vk::BufferCopy copyRegion({}, {}, size);
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
	endSingleTimeCommands(commandBuffer);
}
