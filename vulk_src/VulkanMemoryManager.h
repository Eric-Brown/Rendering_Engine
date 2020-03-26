//
// Created by alexa on 3/25/2020.
//

#ifndef DNDIDEA_VULKANMEMORYMANAGER_H
#define DNDIDEA_VULKANMEMORYMANAGER_H

#include "ExternalHeaders.h"

class VulkanMemoryManager {
public:
	static void Init(vk::Device device, vk::PhysicalDevice physicalDevice);

	static VulkanMemoryManager *getInstance();

	static void Destroy();

	void DestroyImage(vk::Image img, VmaAllocation imgMemory);

	void DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation);

	std::tuple<vk::Buffer, VmaAllocation> initializeStagingBuffer(void *data, size_t dataSize);

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,
	                  vk::Buffer &buffer, VmaAllocation &bufferAllocation);

	void CopyDataToAllocation(void *toCopy, vk::DeviceSize copySize, VmaAllocation allocation);

	std::tuple<vk::Image, VmaAllocation>
	createImage(VkImageCreateInfo imageInfo, VmaAllocationCreateInfo allocationCreateInfo);

	template<typename T>
	std::tuple<vk::Buffer, VmaAllocation>
	createBufferTypeFromVector(std::vector<T> thing, vk::BufferUsageFlags bufferType) {
		vk::DeviceSize bufferSize = sizeof(T) * thing.size();
		auto[stagingBuffer, stagingBufferAllocation] = initializeStagingBuffer(thing.data(), bufferSize);
		vk::Buffer bufferToReturn;
		VmaAllocation allocationToReturn;
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | bufferType, VMA_MEMORY_USAGE_GPU_ONLY,
		             bufferToReturn, allocationToReturn);
//		copyBuffer(stagingBuffer, bufferToReturn, bufferSize); ERROR HERE
		vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
		return std::make_tuple(bufferToReturn, allocationToReturn);
	}

private:
	VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physicalDevice);

	static inline VulkanMemoryManager *vmmInstance{};
	VmaAllocator allocator;
	vk::Device device;
	vk::PhysicalDevice physicalDevice;
	vk::CommandPool commandPool;

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

};

#endif //DNDIDEA_VULKANMEMORYMANAGER_H
