#include "VulkanImageManager.h"

VulkanImageManager::ImageHandleInfo VulkanImageManager::CreateImageFromFile(std::string fName)
{
	auto [rawImgBytes, info] = LoadImageFile(fName);
	auto [stgBuffer, stgAllocation] = VulkanMemoryManager::getInstance()->StageData(rawImgBytes, CalculateImageSize(info));
	stbi_image_free(rawImgBytes);
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	auto [image, dstAllocation] = CreateImageBuffer(info, allocInfo);
	ImageHandleInfo imageInfo{image, dstAllocation, info};
	TransitionImageLayout(imageInfo, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	CopyBufferToImage(stgBuffer, imageInfo);
	VulkanMemoryManager::getInstance()->DestroyBuffer(stgBuffer, stgAllocation);
	return imageInfo;
}

void VulkanImageManager::CopyBufferToImage(vk::Buffer buffer, const ImageHandleInfo &info)
{
	vk::CommandBuffer commandBuffer = VulkanMemoryManager::getInstance()->beginSingleTimeCommands();
	vk::ImageSubresourceLayers subresource{vk::ImageAspectFlagBits::eColor, 0, 0, 1};
	vk::Offset3D imageOffset{0, 0, 0};
	vk::BufferImageCopy region(0, 0, 0, subresource, imageOffset, info.extent);
	commandBuffer.copyBufferToImage(buffer, info.image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	VulkanMemoryManager::getInstance()->endSingleTimeCommands(commandBuffer);
}

void VulkanImageManager::GenerateMipmaps(const ImageHandleInfo &info)
{
	vk::FormatProperties formatProperties{physicalDevice.getFormatProperties(info.format)};
	if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error(TEXTURE_FORMAT_NOT_SUPPORT_BLITTING_MSG);
	}
	vk::CommandBuffer commandBuffer = VulkanMemoryManager::getInstance()->beginSingleTimeCommands();
	vk::ImageMemoryBarrier imgToXferSrcBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead,
											   vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal,
											   VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, info.image,
											   {vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1});
	vk::ImageMemoryBarrier imgToReadOnlyBarrier(vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eShaderRead,
												vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, info.image,
												{vk::ImageAspectFlagBits::eColor, {}, 1, 0, 1});
	int32_t mipWidth = info.extent.width;
	int32_t mipHeight = info.extent.height;
	for (uint32_t i = 1; i < info.imageMipLevels; i++)
	{
		imgToXferSrcBarrier.subresourceRange.baseMipLevel = i - 1;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
									  0, nullptr, 0, nullptr, 1, &imgToXferSrcBarrier);
		const std::array<vk::Offset3D, 2> srcOffset{vk::Offset3D{0, 0, 0},
													vk::Offset3D{mipWidth, mipHeight, 1}};
		const std::array<vk::Offset3D, 2> dstOffset{vk::Offset3D{0, 0, 0},
													vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1,
																 mipHeight > 1 ? mipHeight / 2 : 1, 1}};
		vk::ImageBlit blit({vk::ImageAspectFlagBits::eColor, i - 1, 0, 1}, srcOffset,
						   {vk::ImageAspectFlagBits::eColor, i, 0, 1}, dstOffset);
		commandBuffer.blitImage(info.image, vk::ImageLayout::eTransferSrcOptimal,
								info.image, vk::ImageLayout::eTransferDstOptimal,
								1, &blit, vk::Filter::eLinear);
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
									  {}, 0, nullptr, 0, nullptr, 1, &imgToReadOnlyBarrier);
		mipWidth /= (mipWidth > 1) ? 2 : 1;
		mipHeight /= (mipHeight > 1) ? 2 : 1;
	}
	imgToReadOnlyBarrier.subresourceRange.baseMipLevel = info.imageMipLevels - 1;
	imgToReadOnlyBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	imgToReadOnlyBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
								  0, nullptr, 0, nullptr, 1, &imgToReadOnlyBarrier);
	VulkanMemoryManager::getInstance()->endSingleTimeCommands(commandBuffer);
}

std::tuple<void *, vk::ImageCreateInfo> VulkanImageManager::LoadImageFile(const std::string &fName)
{
	using namespace std;
	int imgWidth, imgHeight, imgChannels;
	stbi_uc *pixels = stbi_load(fName.c_str(), &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);
	if (!pixels)
	{
		ThrowSTBI_Error(fName);
	}
	auto mipLevels = static_cast<uint32_t>(floor(log2((max)(imgWidth, imgHeight)))) + 1;
	vk::Extent3D imgExtent{static_cast<uint32_t>(imgWidth), static_cast<uint32_t>(imgHeight), 1};
	vk::ImageCreateInfo info{{}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm, imgExtent, mipLevels, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, {}, {}, {}};
	return tuple<void *, vk::ImageCreateInfo>(pixels, info);
}

void VulkanImageManager::ThrowSTBI_Error(const std::string &fName)
{
	using namespace std;
	string reason{stbi_failure_reason()};
	string message{"Failed to load given image file: " + fName + "!\nReason given: " + reason};
	throw runtime_error(message);
}

vk::DeviceSize VulkanImageManager::CalculateImageSize(const vk::ImageCreateInfo &info)
{
	uint32_t imgChannels{4};
	//In future can switch using image format from info
	uint32_t bytesPerChannel{1};
	return static_cast<vk::DeviceSize>(info.extent.width * info.extent.height * imgChannels * bytesPerChannel);
}

std::tuple<vk::Image, VmaAllocation> VulkanImageManager::CreateImageBuffer(VkImageCreateInfo info, VmaAllocationCreateInfo allocationInfo)
{
	VkImage temp;
	VmaAllocation alloc;
	vmaCreateImage(VulkanMemoryManager::getInstance()->GetAllocator(), &info, &allocationInfo, &temp, &alloc, nullptr);
	return std::tuple<vk::Image, VmaAllocation>(temp, alloc);
}

void VulkanImageManager::TransitionImageLayout(ImageHandleInfo imageInfo, vk::ImageLayout fromLayout, vk::ImageLayout toLayout)
{
	vk::ImageAspectFlags resourceAspectMask = DetermineTransitionResourceAccessFlags(imageInfo.format, toLayout);
	vk::ImageSubresourceRange imageSubresourceRange(resourceAspectMask, 0u, imageInfo.imageMipLevels, 0u, 1u);
	try
	{
		auto [srcMask, srcPipe] = DetermineTransitionBarrierSourceFlags(fromLayout);
		auto [dstMask, dstPipe] = DetermineTransitionBarrierDestinationFlags(toLayout);
		vk::ImageMemoryBarrier imageTransitionCompletionBarrier{srcMask, dstMask, fromLayout, toLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
																imageInfo.image, imageSubresourceRange};
		vk::CommandBuffer commandBuffer = VulkanMemoryManager::getInstance()->beginSingleTimeCommands();
		commandBuffer.pipelineBarrier(srcPipe, dstPipe, {}, 0, nullptr, 0, nullptr, 1, &imageTransitionCompletionBarrier);
		VulkanMemoryManager::getInstance()->endSingleTimeCommands(commandBuffer);
	}
	catch (const std::exception &)
	{
		ThrowInvalidTransitionError(fromLayout, toLayout);
	}
}

vk::ImageAspectFlags VulkanImageManager::DetermineTransitionResourceAccessFlags(vk::Format srcFormat, vk::ImageLayout to)
{
	vk::ImageAspectFlags accessFlags{vk::ImageAspectFlagBits::eColor};
	if (to == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		accessFlags = HasStencilComponent(srcFormat) ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;
	}
	return accessFlags;
}

std::tuple<vk::AccessFlags, vk::PipelineStageFlags> VulkanImageManager::DetermineTransitionBarrierDestinationFlags(vk::ImageLayout to)
{
	using namespace std;
	tuple<vk::AccessFlags, vk::PipelineStageFlags> dstFlags{};
	switch (to)
	{
	case vk::ImageLayout::eTransferDstOptimal:
		get<0>(dstFlags) = vk::AccessFlagBits::eTransferWrite;
		get<1>(dstFlags) = vk::PipelineStageFlagBits::eTransfer;
		break;
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		get<0>(dstFlags) = vk::AccessFlagBits::eShaderRead;
		get<1>(dstFlags) = vk::PipelineStageFlagBits::eFragmentShader;
		break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		get<0>(dstFlags) = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		get<1>(dstFlags) = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		break;
	default:
		throw runtime_error("");
		break;
	}
	return dstFlags;
}

std::tuple<vk::AccessFlags, vk::PipelineStageFlags> VulkanImageManager::DetermineTransitionBarrierSourceFlags(vk::ImageLayout from)
{
	using namespace std;
	tuple<vk::AccessFlags, vk::PipelineStageFlags> srcFlags{};
	switch (from)
	{
	case vk::ImageLayout::eUndefined:
		get<1>(srcFlags) = vk::PipelineStageFlagBits::eTopOfPipe;
		break;
	case vk::ImageLayout::eTransferDstOptimal:
		get<0>(srcFlags) = vk::AccessFlagBits::eTransferWrite;
		get<1>(srcFlags) = vk::PipelineStageFlagBits::eTransfer;
		break;
	default:
		throw runtime_error("");
		break;
	}
	return srcFlags;
}

void VulkanImageManager::ThrowInvalidTransitionError(vk::ImageLayout from, vk::ImageLayout to)
{
	using namespace std;
	string fromName =  GetImageLayoutName(from);
	string toName = GetImageLayoutName(to);
	string message{"Unsupported layout transition.\nFrom: " + fromName + "\nTo: " + toName};
	throw invalid_argument(message);
}

std::string VulkanImageManager::GetImageLayoutName(vk::ImageLayout layout)
{
	using namespace std;
	string layoutName{};
	switch (layout)
	{
	case vk::ImageLayout::eUndefined:
		layoutName = "Undefined";
		break;
	case vk::ImageLayout::eTransferDstOptimal:
		layoutName = "Transfer Destination Optimal";
		break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		layoutName = "Depth Stencil Attachment Optimal";
		break;
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		layoutName = "Shader Read Only Optimal";
		break;
	default:
		layoutName = "Unrecognized layout name";
		break;
	}
	return layoutName;
}

void VulkanImageManager::DestroyImageView(vk::ImageView view)
{
	logicalDevice.destroyImageView(view);
}

void VulkanImageManager::DestroySampler(vk::Sampler sampler)
{
	logicalDevice.destroySampler(sampler);
}

VulkanImageManager *VulkanImageManager::getInstance()
{
	return vimInstance;
}

void VulkanImageManager::Destroy()
{
	delete vimInstance;
}

void VulkanImageManager::DestroyImage(vk::Image img, VmaAllocation imgMemory)
{
	vmaDestroyImage(VulkanMemoryManager::getInstance()->GetAllocator(), img, imgMemory);
}

void VulkanImageManager::Init(vk::Device device, vk::PhysicalDevice physDevice)
{
	if (!vimInstance)
	{
		vimInstance = new VulkanImageManager(device, physDevice);
	}
}

VulkanImageManager::VulkanImageManager(vk::Device device, vk::PhysicalDevice physDevice)
	: logicalDevice{device}, physicalDevice{physDevice}
{
}

bool VulkanImageManager::HasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::ImageView VulkanImageManager::CreateImageView(vk::ImageViewCreateInfo info)
{
	return logicalDevice.createImageView(info);
}

vk::Sampler VulkanImageManager::CreateImageSampler(vk::SamplerCreateInfo info)
{
	return logicalDevice.createSampler(info);
}
