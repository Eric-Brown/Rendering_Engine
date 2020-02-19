# Swap Chain Recreation

While we currently have a working application right now, it is unable to handle window resizes.
Therefore, we need to fix this.

Thankfully, to do so is not too bad.
We create a new function aptly named recreateSwapChain.
This waits for device idle, cleans up the old swap chain and then recreates everything from scratch...mostly.

The swap chain is at the heart of most of the objects that we are utilizing right now.
We will have to recreate:
1. The swap chain
1. The image views
1. The Render Pass
1. The Graphics Pipeline
1. The Framebuffers
1. The CommandBuffers

Now, it is possible to give the new swap chain the handle to the old swap chain so that draw commands can continue while all of this is happening.
I'm currently not doing this, and the results can be seen as you continually resize the window.
There is a lot of slowness that happens.
Hopefully resizing the window is not a common event.

Now, in order to tell if the framebuffer has changed sizes, there are several ways to do so.
The first is that when you fetch an image or when you present an image, special values can be returned:
- VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface and can no longer be used for rendering. Usually happens after a window resize.
- VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched exactly.

Looking for these values after issuing the commands, you can handle the swap chain recreation there.

Alternatively, you can pass a handling function to glfw so that when the window is resized, you can set a sentinel value.

Finally, for minimization, just look for when height or width are zero. If that is the case, just wait for another resize that has a nonzero area.
Pretty straight forward.
