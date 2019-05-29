# Presentation Setup Notes

In order to actually present images/graphics to the user, you need to first set up a presentation to write to.

For the purposes of this project this means:

1. Creating a **Surface** to render to and a **Presentation Queue** to send commands to
2. Creating a **Swap Chain** for that surface
3. Creating **Image Views** that are for use with the **Swap Chain**

## Surface

For this project, the surface is the area of the screen represented by the window created by GLFW.
We use GLFW to create the surface such that the surface is connected to a given window, and a given Vulkan instance.
Without GLFW this would have to be done in a platform dependent way, using platform dependent structs.

When you will be presenting to a surface, it is important to verify that a given device supports presenting to a given surface.
This is done through:
```objectivec
VkBool32 presentSupport = false;
vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
```

Correspondingly, when creating the logical device along with the graphics queue, a present queue needs to be created.
This is done in a similar fashion to how the graphics queue is created.
Remember to give the creation object the correct number of queues.

## Swap Chain

The swap chain is essentially the queue of images to be presented to the screen. 
Vulkan does not have a frame buffer by default, nor a swap chain that connects to that buffer.
They must be created.
The swap chain is considered an extension, and therefore it must be checked if it is supported when selecting a physical device.
There is a `#define` that contains the extension name.
To enable these extensions, pass the information such as names and extension count to the logical device creation object.

Once the swap chain is enabled (or while selecting a possible device), you need to query what **capabilities**, **formats**, and **present modes** are supported.
This can be done through a call to various `vkGetPhysicalDeviceSuface...KHR`, providing the appropriate arguments.

1. **Capabilities**
	- Includes information about the swap extent; i.e. the resolution of images in the swap chain
		- The extent has information about what the current extent is (usually the window size), what the minimum size is, and what the maximum size is.
		When choosing the extent, you need to clamp it between min, max, and current.
		- Some window managers here allow us to change the settings to uint32_t max.
	- Also includes information about the minimum and maximum supported image count.
		- Rule of thumb is +1 to minimum, being clamped by maximum.
		- Maximum is 0 if there is no set max. CHECK FOR THIS BEFORE CLAMPING.
2. **Formats**
	- The image format, such as sRGB or lRGB. Best choice will be `VK_FORMAT_B8G8R8A8_UNORM` for most purposes.
	- `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` is part of the colorspace. This is the linear/nonlinear selection.
	- If you cannot find a desired format, just choose the first available. 
	- If `VK_FORMAT_UNDEFINED`, what luck! it's letting you choose!
3. **Present Modes** 
	- Mode defines have the prefix: `VK_PRESENT_MODE_`
	- `IMMEDIATE` is no buffer and images go directly to the screen. This can result in tearing.
	- `FIFO` Is a queue and is the *only mode guaranteed to be available*. 
	If the queue is full, the program is blocked until the image is swapped into the frame buffer.
	- `FIFO_RELAXED` Is similar to `FIFO`, but instead of waiting for a vsync before adding an image, the image is added ASAP. 
	This can also result in tearing if the draw rate is slower than frame rate.
	- `MAILBOX` Is a variation of `FIFO`. Instead of blocking to add an image, the image is instead replaced.
	This can be used to implement triple buffering and decrease latency as well as avoid tearing.

Once you have chosen the various parameters, creation is once again filling out a structure and placing in the appropriate data.
Of note:
- imageUsage pertains to what operations will be used with the images.
- If you have more than one queue operating with the images, this needs to be included, along with a choice of `CONCURRENT` or `EXCLUSIVE` sharing modes.
- Certain transforms can be applied to images in the chain if supported.
- compositeAlpha says how to blend with other windows.
- **Swap chains can become invalidated!** If this is the case, you have to recreate the swap chain from scratch.
When you do so, you can provide the handle to the old swap chain that is being replaced.
- Don't forget to destroy the swap chain when you are done.

Finally, once creation is done, you need handles to the array of `VkImages` which the swap chain contains.
Get these through a couple of calls to `vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data())`
Also! Store the format and extents as these will be needed by other functions.

## Image Views

While we may have handles to `VkImages`, we can't render to them. 
For that we need a `VkImageView`. 
This describes how to access the image, and what part(s) of the image to access. 
(e.g. if the image is a 2D texture depth texture without mipmapping levels)
 
 For this project, `createImageViews` simply creates a basic view of the swap chain images so that they can be color targets later on.
 As with the other parts, the handles to the views are stored.
 Creation of an ImageView is done the same as with all the other vulkan objects: filling out a `VkImageViewCreateInfo` structure.
 
 Of note for the structure:
 - viewType pertains to 1D, 2D, 3D and/or cube maps
 - components allows you to swizzle parts if desired. Or to set a constant value.
 - subresourceRange describes the images purpose and which part of the image is to be accessed.
 This can be useful if for instance you are working on a stereographic application.
- Remember to clean up allocated image views.
