#ifndef RENDER_ENGINE_VIM
#define RENDER_ENGINE_VIM
#include "VulkanMemoryManager.h"

class VulkanImageManager
{
public:
	struct ImageHandleInfo
	{
		vk::Image image;
		VmaAllocation allocation;
		vk::Format format;
		uint32_t imageMipLevels;
		vk::Extent3D extent;
		ImageHandleInfo(vk::Image image, VmaAllocation alloc, const vk::ImageCreateInfo &createInfo)
			: image{image}, allocation{alloc}, format{createInfo.format}, imageMipLevels{createInfo.mipLevels}, extent{createInfo.extent}
		{
		}
	};

private:
	static inline VulkanImageManager *vimInstance{};
	vk::Device logicalDevice;
	vk::PhysicalDevice physicalDevice;
	VulkanImageManager(vk::Device device, vk::PhysicalDevice physDevice);
	std::tuple<void *, vk::ImageCreateInfo> LoadImageFile(const std::string& fName);
	vk::DeviceSize CalculateImageSize(const vk::ImageCreateInfo &info);
	vk::ImageAspectFlags DetermineTransitionResourceAccessFlags(vk::Format srcFormat, vk::ImageLayout to);
	std::tuple<vk::AccessFlags, vk::PipelineStageFlags> DetermineTransitionBarrierDestinationFlags(vk::ImageLayout to);
	std::tuple<vk::AccessFlags, vk::PipelineStageFlags> DetermineTransitionBarrierSourceFlags(vk::ImageLayout from);
	std::string GetImageLayoutName(vk::ImageLayout layout);
	void ThrowInvalidTransitionError(vk::ImageLayout from, vk::ImageLayout to);
	void CopyBufferToImage(vk::Buffer buffer, const ImageHandleInfo &info);
	void ThrowSTBI_Error(const std::string &fName);

public:
	ImageHandleInfo CreateImageFromFile(std::string fName);
	std::tuple<vk::Image, VmaAllocation> CreateImageBuffer(VkImageCreateInfo info, VmaAllocationCreateInfo allocationInfo);
	vk::ImageView CreateImageView(vk::ImageViewCreateInfo info);
	vk::Sampler CreateImageSampler(vk::SamplerCreateInfo info);
	void GenerateMipmaps(const ImageHandleInfo &info);
	void DestroyImage(vk::Image img, VmaAllocation imgMemory);
	void DestroyImageView(vk::ImageView view);
	void DestroySampler(vk::Sampler sampler);
	void TransitionImageLayout(ImageHandleInfo imageInfo, vk::ImageLayout fromLayout, vk::ImageLayout toLayout);
	static VulkanImageManager *getInstance();
	static bool HasStencilComponent(vk::Format format);
	static void Init(vk::Device device, vk::PhysicalDevice physDevice);
	static void Destroy();
};
#endif // RENDER_ENGINE_VIM