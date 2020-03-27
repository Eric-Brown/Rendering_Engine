//
// Created by alexa on 3/25/2020.
//

#ifndef DNDIDEA_VULKANMEMORYMANAGER_H
#define DNDIDEA_VULKANMEMORYMANAGER_H

#include "ExternalHeaders.h"
static const char *const TEXTURE_FORMAT_NOT_SUPPORT_BLITTING_MSG = "Texture image format does not support linear blitting!";
class VulkanMemoryManager
{
public:
  static void Init(vk::Device device, vk::PhysicalDevice physDevice,
                   vk::CommandPool pool, vk::Queue queue);

  static VulkanMemoryManager *getInstance();

  static void Destroy();

  void DestroyImage(vk::Image img, VmaAllocation imgMemory);

  void DestroyBuffer(vk::Buffer buff, VmaAllocation buffAllocation);

  void DestroyImageView(vk::ImageView view);
  
  void DestroySampler(vk::Sampler sampler);

  std::tuple<vk::Buffer, VmaAllocation>
  initializeStagingBuffer(void *data, size_t dataSize);

  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                    VmaMemoryUsage memoryUsage, vk::Buffer &buffer,
                    VmaAllocation &bufferAllocation);

  void CopyDataToAllocation(void *toCopy, vk::DeviceSize copySize,
                            VmaAllocation allocation);

  std::tuple<vk::Image, VmaAllocation>
  createImage(VkImageCreateInfo imageInfo,
              VmaAllocationCreateInfo allocationCreateInfo);

  vk::ImageView CreateImageView(vk::ImageViewCreateInfo info);

  vk::Sampler CreateImageSampler(vk::SamplerCreateInfo info);

  template <typename T>
  std::tuple<vk::Buffer, VmaAllocation>
  createBufferTypeFromVector(std::vector<T> thing,
                             vk::BufferUsageFlags bufferType)
  {
    vk::DeviceSize bufferSize = sizeof(T) * thing.size();
    auto [stagingBuffer, stagingBufferAllocation] =
        initializeStagingBuffer(thing.data(), bufferSize);
    vk::Buffer bufferToReturn;
    VmaAllocation allocationToReturn;
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | bufferType,
                 VMA_MEMORY_USAGE_GPU_ONLY, bufferToReturn, allocationToReturn);
    copyBuffer(stagingBuffer, bufferToReturn, bufferSize);
    DestroyBuffer(stagingBuffer, stagingBufferAllocation);
    // vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
    return std::make_tuple(bufferToReturn, allocationToReturn);
  }

  vk::DeviceSize GetAllocationSize(VmaAllocation allocation);

  std::tuple<vk::Image, VmaAllocation> CreateImageFromData(void *data, vk::DeviceSize size, vk::ImageCreateInfo info, VmaMemoryUsage usage);

private:
  VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physDevice,
                      vk::CommandPool pool, vk::Queue queue);
  static inline VulkanMemoryManager *vmmInstance{};
  VmaAllocator allocator;
  vk::Device logicalDevice;
  vk::PhysicalDevice physicalDevice;
  vk::CommandPool commandPool;
  vk::Queue graphicsQueue;

  void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                  vk::DeviceSize size);

  vk::CommandBuffer beginSingleTimeCommands();

  void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

  void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout, uint32_t inMipLevels);

  void copyBufferToImage(vk::Buffer buffer, vk::Image image, vk::ImageCreateInfo info);
  void generateMipmaps(vk::Image image, vk::ImageCreateInfo info);
  bool hasStencilComponent(vk::Format format);
};

#endif // DNDIDEA_VULKANMEMORYMANAGER_H
