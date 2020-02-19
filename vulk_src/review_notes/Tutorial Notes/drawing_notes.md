# Setup and Implementation of Drawing

Now that we have our pipeline, our vkImage destinations, and the various other components, we can finally move to drawing.
In this section, we take care of a final few necessary components, and then move onto drawing.
The steps can be broken down as such:
1. Create a framebuffer
2. Create a command pool and its associated command buffers.
3. Use the command pool and actually draw stuff to the screen.
4. Implement a drawFrame function.

## Framebuffer Creation

We have stated several times throughout our notes that the graphics output destination is ultimately the framebuffer, but we haven't actually got a framebuffer yet.
While you might have thought that our swap chain was actually our frame buffer, it's not! 
Because the swap chain is an extension and not a core part of Vulkan, the Framebuffer (which is part of core Vulkan) cannot assume that you are using a swap chain.
Also, since we are using multiple images in our framebuffer, we will need to have multiple framebuffer handles.
 
For this project, we take care of framebuffer creation in a loop.
In this loop, we select one of the `imageviews`, and fill out a `vkFramebufferCreateInfo` structure.
We associate this framebuffer with a render pass object, any attachments (this is the ImageView in our case), the width and height to be rendered, and finally the layers which specifies the number of layers in the image arrays.
After a call to `vkCreateFramebuffer` we will have our framebuffer allocated.
Remember to destroy this object at the end.

## Command Pool and Command Buffers

Unlike OpenGL, operations are not done through API calls.
Instead, commands are queued up and then asynchronously executed.
This is advantageous as you now can have a much easier time interacting with the graphics API in a multithreaded environment.

### Command Pool

The command pool is what manages the memory that is used to store the buffers and command buffers are allocated from them.
Since this is a Vulkan object that we must create, keep a handle to it so that we can destroy it later. 
Additionally, the command pool can be used in other functions.

The command pool interacts with the other queues that have been allocated and can only allocate command buffers that are submitted to a SINGLE type of queue.
Therefore, when filling in the `VkCommandPoolCreateInfo`, we will pass the index of the graphics family queue.
The flags portion of the command buffer can specify whether or not commands are frequently rerecorded and/or what circumstances to expect.
There are two possible flags:
- `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
- `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together

### Command Buffer Allocation

Now that we have a command pool, we can allocate the command buffers which actually will store the commands that are sent.
We are allocating several command buffers since each command buffer will interact with an individual `ImageView` target.
While we will end up configuring the allocated buffers within a loop, the buffers themselves will be allocated through a single call to `vkAllocateCommandBuffers`.
A `VkCommandBufferAllocateInfo` structure is used to specify the information needed.
Of note: level refers to whether a set of buffers are primary(can be submitted to a queue for execution, but never referenced) or secondary(cannot be submitted directly, but can be referenced).
Secondary buffers can be useful if you find yourself issuing common operations over and over again.

Now that the buffers are allocated, they must have their commands recorded.
Since we are just rendering a simple hardcoded triangle to the screen, there is no need to rerecord.
We begin by using a `VkCommandBufferBeginInfo` structure to specify details about the usage of the command buffer.
The only field of interest is the `flags` field. pInheritanceInfo is for secondary queues.
This is what tells the API how we're going to use the buffer.

The available flags:
- `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`: The command buffer will be rerecorded right after executing it once.
- `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`: This is a secondary command buffer that will be entirely within a single render pass.
- `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT`: The command buffer can be resubmitted while it is also already pending execution.

#### Starting a Render Pass

This is issued through a `VkRenderPassBeginInfo` structure.
This is where we are specifying a particular framebuffer target.
We will also specify our particular render pass object.
In addition, we specify the size of the area to be rendered to once more.
Here we can also specify an arrayof clear colors to use for when the clear operation is done.

Once the structure has all the needed information, we record the pass with a `vkCmdBeginRenderPass` call.
Of note: all commands that are recorded into the buffer will have a similar structure to the above. 
In addition, these functions will all return void, so be sure to check for errors at the end.
Oh, and this functions last parameter has the following allowed values:
- VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
- VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
    
#### Basic Drawing Commands

Now, we can record the binding of the pipeline we wish to use with: `vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);`
Once the pipeline is set up and the pass has been set up as well, we can issue the precious draw command: `vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);`

Parameters, aside from the command buffer:

- vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
- instanceCount: Used for instanced rendering, use 1 if you're not doing that.
- firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
- firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex. 

Finally, you end the render pass setup with a call to `vkCmdEndRenderPass(commandBuffers[i]);`
Followed by a call to `vkEndCommandBuffer(commandBuffers[i])` for finalizing the recording.

## DrawFrame / Rendering and Presentation

Our drawFrame function will perform the following:
- Acquire an image from the swap chain
- Execute the command buffer with that image as attachment in the framebuffer
- Return the image to the swap chain for presentation

When performing these operations, it is important to note that while these individual things may be a single function call, they are asynchronous operations.
This means that we need to synchronize our operations, otherwise we will have race conditions.
Vulkan provides two synchronization objects: fences and semaphores.

### Fences and Semaphores

Semaphores work exactly how semaphores operate on CPUs, but in vulkan the semaphores only exist on the GPU.
Therefore, they are useful for synchronizing GPU operations with other GPU operations.
To create them you will use a `VkSemaphoreCreateInfo` structure.
This structure only needs the `sType` field populated.
Then a call to `vkCreateSemaphore` will do the magic. 
Just remember to destroy these semaphores at the end.

For using semaphores, there are several functions which take arrays of semaphores on which to wait, and also arrays of semaphores on which to signal for completion.
We use this in our acquire image function, so that we can signal the semaphore once an image has become available.
Other functions will then wait on this semaphore and also signal to a different semaphore that a render pass has been completed and an image is ready to be presented.

Fences are synchronization objects for halting the CPU while the GPU does work.
This is useful for synchronizing the operations that are sent to the GPU.
In our case, we use it to ensure that there are only a few frames "on the fly".
Otherwise, we would issue draw commands faster than the GPU can perform, and our memory usage would balloon out of control.
To create a fence, we need to create a `VkFenceCreateInfo` structure and have a matching call to a create function.
Remember to destroy the fence when we are done.

Of note for fences: They do not start out flagged, and you might encounter a deadlock as a result.
Therefore, we are using the create info to set the flags field to "signaled" so that we do not deadlock.

Fences are waited for using a `vkWaitForFences` function, which can wait for any, or all of the fences in a given array.
Once a fence is acquired, you can set the fence to the unsignaled state by using the `vkResetFences` function.

### Rendering and Presentation Methods

The logic that drawFrame follows is this: acquire an image from the swap chain.
Find the command buffer that operates on that particular image, and submit it to the queue.
Once the commands in the command buffer are executed, submit the image to the present queue.

The first part we have roughly covered in our synchronization objects section: it is a call to `vkAcquireNextImageKHR`.

To submit and synchronize to the queue, we have to use a `VkSubmitInfo` structure.
We set an array of semaphores to wait on to begin execution, and we also set an array of `VkPipelineStageFlags` to also wait on.
In our case, we are waiting for `VK PIPELINE STAGE COLOR ATTACHMENT OUTPUT BIT`.
This is because we need to wait until both the image is available and that we are at the color output stage.

> Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.

Next, we specify a command buffer array to execute (in our case only the one buffer that is associated with the current image buffer).
Finally, we specify an array of semaphores to signal once the operations are completed.
To get the ball rolling, we make a call to `vkQueueSubmit`.

#### Subpass Dependencies

Subpasses in a render pass automatically take care of image layout transitions, and these transitions are controlled by subpass dependencies.
These dependencies specify memory and execution dependencies between subpasses.
While we only have one subpass right now, there are operations right before and after this subpass which are also considered subpasses.
These builtin subpasses unfortunately have a race condition right at the start: we need to transition the image, but we may not have acquired this image yet!

There are two ways we can deal with this problem: we can change the wait stages in the submit info to be the TOP OF PIPE stage to ensure that passes don't begin until we have an image available.
Alternatively, we can make the render pass wait for the COLOR ATTACHMENT OUTPUT BIT.
This second option requires using subpass dependencies which is why it is chosen.

We begin by creating a `VkSubpassDependency` structure.
Our source subpass is `VK SUBPASS EXTERNAL` since it is one of the implicit subpasses.
The destination subpass is the index of the subpass we wish to have the dependency.
Of note: the source must be of a lower index than the destination to prevent cyclical dependencies.

We then specify the stage on which we have a dependency and the operations that we are waiting on.
This will be the color attachment output bit.
Next, we specify which operations and stages in the destination subpass have the dependency.
In our case, it is also the color attachment output bit, and the accessmask will be the color attachment read/write bits.
In summary, these settings will prevent the color attachment output stage in our subpass from reading and writing from the color attachment until the external subpass has completed its color attachment output stage.

We add this information to our render pass info structure when we are creating our render pass initially.

### Presentation

Finally, our last step of getting something to show on screen.
We create a `VkPresentInfoKHR` structure, and hand it the semaphores to wait on, the swap chains from which to grab an image, along with the indices of those images in the swap chains.
The pResults field is unused in our case as we have only one swap chain to worry about.
If we had more, we can capture the individual results to see if a particular one failed.
We only will check with the return value of `VkQueuePresentKHR`.

When we exit the main loop, we add a call to `vkDeviceWaitIdle` so that we do not destroy objects that may or may not still be in use.

## Frames in Flight

I have already stated above our need for fences to prevent our command buffer from growing out of control.
Here I will state all the other things needed to ensure that we only have a few frames "in flight".

Obviously, we add a class member to describe which of our in flight frames to use,and we also add semaphores for each of the frames.
The purpose of the added fences is to ensure that we are not trying to use frame X's objects while frame X is still in flight.
When we get to our `vkQueueSubmit` function call, we add the fence in question as a parameter.
This means that when the QueueSubmit has *completed*, the fence is signaled.

In turn, since we wait for the fence to be signaled before trying to use any particular frame object, we wait for the signal first thing in our drawFrame function.
After which, we reset the fence so that we can ensure it is toggled **only** when the frame is completed.
Here again I state: make sure to destroy everything that is created. No exceptions.
