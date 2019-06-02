# Depth Buffering

When you add in new geometry to the scene, as it currently stands, the last drawn thing wins.
So if you were to add something into the far back, if it were drawn last, it would show up ON TOP of everything else.
Of course, our current program only dealt with 2D position vectors, but we are adding whole new dimension in this section.
To fix the bad draw order there are two solutions:
1. Reorder the declaration of geometry such that the geometry that is most in the "foreground" is drawn last.
2. Use a depth buffer to keep track of how far "back" a drawn pixel was.

While the first solution might be your only choice if you're dealing with transparency, the second is far more flexible.

## Creating a Depth Buffer

The depth buffer is basically an image attachment.
The depth values for a given pixel is stored, and if the pixel to be drawn is further away than what was last written, it is discarded.

Because the depth buffer is literally an image attachment, it is created like all the other images thus far:
- Create a vulkan image handle
- Allocate memory for the image
- Get a view into the image

Our depth buffer will match the extents of the current swap chain format.
Of note though: the depth buffer has a special image format.
It is something along the lines of:
- VK_FORMAT_D32_SFLOAT: 32-bit float for depth
- VK_FORMAT_D32_SFLOAT_S8_UINT: 32-bit signed float for depth and 8 bit stencil component
- VK_FORMAT_D24_UNORM_S8_UINT: 24-bit float for depth and 8 bit stencil component

When creating the depth buffer, you need to select the most optimal depth format, and you need to ensure that this depth format is supported.
To that end, there is a `findSupportedFormat` function, the details of which you can look on your own time.
Suffice it to say, you want to make sure that the features match whether or not tiling is linear or optimal.

In our case, we look for D24 S8 UINT format, tiling optimal, and with a stencil attachment.
Even though we aren't using a stencil buffer, it is an option.

Once we know the format for our depth format image, we create it, and allocate the memory that the image needs.
Then we create a view into that image, but make sure that the image aspect bit is set to depth bit.

Also: transition the image layout to be optimal for depth stencil attachment. (More hideous/obtuse code was written for handling the logic for this)

## Attaching a Depth Buffer

The depth buffer is an attachment, just like the image attachment to the framebuffer that we use.
Therefore, we add a new attachment description like so:
```objectivec
VkAttachmentDescription depthAttachment = {};
depthAttachment.format = findDepthFormat();
depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
```

We add an attachment reference, and then add the reference to the subpass as a pDepthStencilAttachment.
We also pass all the attachments to the renderPassInfo structure.

Finally, we attach the depth buffer view to the framebuffer.

### More Details

We need to add a clear value to the attachment during command buffer creation.
In our case, we are setting it to 1 since that is the furthest possible value for the depth buffer.
(Unless you change the state in the state info.)

SPEAKING OF STATE INFO:
While the attachent is configured for the render pass, it is not configured for use with the graphics pipeline.
We create a `kPipelineDepthStencilStateCreateInfo depthStencil`, fill in the information (e.g. what comparison operator will be used).
Finally, we hand the state info to the pipeline info structure.

FINALLY FINALLY, you need to manage the memory and allocations during a resize with all the other swap chain stuff.
So be sure to allocate/deallocate as necessary for resizes.

