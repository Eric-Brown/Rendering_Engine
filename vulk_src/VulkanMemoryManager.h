//
// Created by alexa on 3/25/2020.
//

#ifndef DNDIDEA_VULKANMEMORYMANAGER_H
#define DNDIDEA_VULKANMEMORYMANAGER_H


#include <vk_mem_alloc.h>
#include "Vertex.h"

class VulkanMemoryManager {
public:
	static void Init(vk::Device device, vk::PhysicalDevice physicalDevice);

	static VulkanMemoryManager *getInstance();

	static void Destroy();

	void DestroyImage(vk::Image img, VmaAllocation imgMemory);

	void DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation);

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,
	                  vk::Buffer &buffer, VmaAllocation &bufferAllocation);

	void CopyDataToAllocation(void* toCopy, vk::DeviceSize copySize, VmaAllocation allocation);

	std::tuple<vk::Image, VmaAllocation> createImage(VkImageCreateInfo imageInfo, VmaAllocationCreateInfo allocationCreateInfo);

	template<typename T>
	std::tuple<vk::Buffer, VmaAllocation> createBufferTypeFromVector(std::vector<T> thing,
	                                                                 vk::BufferUsageFlags bufferType);

private:
	VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physicalDevice);

	static VulkanMemoryManager *vmmInstance;
	VmaAllocator allocator;

//	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
	std::tuple<vk::Buffer, VmaAllocation> initializeStagingBuffer(void *data, size_t dataSize);
};


#endif //DNDIDEA_VULKANMEMORYMANAGER_H
