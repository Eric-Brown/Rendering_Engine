#ifndef RENDER_ENGINE_VIM
#define RENDER_ENGINE_VIM
#include "VulkanMemoryManager.h"

class VulkanImageManager
{
private:
	static inline VulkanImageManager *vimInstance{};
	vk::Device logicalDevice;
	vk::PhysicalDevice physicalDevice;

	VulkanMemoryManager(vk::Device device, vk::PhysicalDevice physDevice);

	std::tuple<void *, vk::ImageCreateInfo> LoadImageFile(std::string fName);
	vk::DeviceSize CalculateImageSize(const vk::ImageCreateInfo &info);
	struct ImageHandleInfo
	{
		vk::Image image;
		vk::Format format;
		uint32_t imageMipLevels;
		vk::Extent3D extent;
		ImageHandleInfo(vk::Image image, const vk::ImageCreateInfo &createInfo)
			: image{image}, format{createInfo.format}, imageMipLevels{createInfo.mipLevels}, extent{createInfo.extent}
		{
		}
	};
	void TransitionImageLayout(ImageHandleInfo imageInfo, vk::ImageLayout fromLayout, vk::ImageLayout toLayout);
	vk::AccessFlags DetermineTransitionResourceAccessFlags(vk::Format srcFormat, vk::ImageLayout to);
	std::tuple<vk::AccessFlags, vk::PipelineStageFlags> DetermineTransitionBarrierDestinationFlags(vk::ImageLayout to);
	std::tuple<vk::AccessFlags, vk::PipelineStageFlags> DetermineTransitionBarrierSourceFlags(vk::ImageLayout from);
	std::string GetImageLayoutName(vk::ImageLayout layout);
	void ThrowInvalidTransitionError(vk::ImageLayout from, vk::ImageLayout to);
	void CopyBufferToImage(vk::Buffer buffer, const ImageHandleInfo &info);
	bool HasStencilComponent(vk::Format format);

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
	ImageHandleInfo CreateImageFromFile(std::string fName);
	std::tuple<vk::Image, VmaAllocation> CreateImageBuffer(const vk::ImageCreateInfo &info, const VmaAllocationCreateInfo &allocationInfo);
	vk::ImageView CreateImageView(vk::ImageViewCreateInfo info);
	vk::Sampler CreateImageSampler(vk::SamplerCreateInfo info);
	void GenerateMipmaps(const ImageHandleInfo &info);
	void DestroyImage(vk::Image img, VmaAllocation imgMemory);
	void DestroyImageView(vk::ImageView view);
	void DestroySampler(vk::Sampler sampler);

	VulkanImageManager *getInstance();
	static void Init(vk::Device device, vk::PhysicalDevice physDevice);
	static void Destroy();
};
#endif // RENDER_ENGINE_VIM