//
// Created by alexa on 3/25/2020.
//

#ifndef DNDIDEA_VULKANMEMORYMANAGER_H
#define DNDIDEA_VULKANMEMORYMANAGER_H

#include "ExternalHeaders.h"
static const char *const TEXTURE_FORMAT_NOT_SUPPORT_BLITTING_MSG = "Texture image format does not support linear blitting!";
class VulkanMemoryManager
{
private:
  VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physDevice, vk::CommandPool pool, vk::Queue queue);
  static inline VulkanMemoryManager *vmmInstance{};
  VmaAllocator allocator;
  vk::Device logicalDevice;
  vk::PhysicalDevice physicalDevice;
  vk::CommandPool commandPool;
  vk::Queue graphicsQueue;
  void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

public:
  static void Init(vk::Device device, vk::PhysicalDevice physDevice, vk::CommandPool pool, vk::Queue queue);
  static VulkanMemoryManager *getInstance();
  static void Destroy();
  VmaAllocator GetAllocator();
  void DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation);
  std::tuple<vk::Buffer, VmaAllocation> initializeStagingBuffer(void *data, size_t dataSize);
  vk::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
  std::tuple<vk::Buffer, VmaAllocation> StageData(void *data, size_t dataSize);
  std::tuple<vk::Buffer, VmaAllocation> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);
  void CopyDataToAllocation(void *toCopy, vk::DeviceSize copySize, VmaAllocation allocation);
  template <typename T>
  std::tuple<vk::Buffer, VmaAllocation> createBufferTypeFromVector(std::vector<T> thing, vk::BufferUsageFlags bufferType)
  {
    vk::DeviceSize bufferSize = sizeof(T) * thing.size();
    auto [stagingBuffer, stagingBufferAllocation] =
        initializeStagingBuffer(thing.data(), bufferSize);
    auto [bufferToReturn, allocationToReturn] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | bufferType, VMA_MEMORY_USAGE_GPU_ONLY);
    copyBuffer(stagingBuffer, bufferToReturn, bufferSize);
    DestroyBuffer(stagingBuffer, stagingBufferAllocation);
    // vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
    return std::make_tuple(bufferToReturn, allocationToReturn);
  }

  vk::DeviceSize GetAllocationSize(VmaAllocation allocation);
};

#endif // DNDIDEA_VULKANMEMORYMANAGER_H
