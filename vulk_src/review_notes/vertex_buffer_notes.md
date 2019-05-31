# Buffers and You!

While it is simple to have hard coded vertices in your shaders, anything but a trivial program will require you to make use of vertex buffers and index buffers.

For this part of the project, we introduce the Vertex Buffer, the Index Buffer, device memory allocation, and device memory transfer operations.
In addition, we quickly introduce the changes necessary in order for shaders to interact with the buffers.

## Shaders and Buffers

While previous versions of our shaders "communicated" by using the same location for 'in' and 'out' variables, we need for the vertex buffer to recieve data and not "generate" it.
Therefore, we specify two 'in' variables to the vertex shader: `in vec2 inPosition` and `in vec3 inColor`.
In the shaders `main` function, we set `gl_Position` to an upgraded position with a w value of 1. 
We also pass off fragColor the inColor value.

For these `in` variables, we use `layout(location=...)` to specify the indices to the inputs so that they can be referenced in our code.
Of note: some variable types take up multiple location indices, so be careful not to start the next index too soon.

## Setting Vertex Data

Now that we have our shader ready to receive data, we need to provide it that data.
In our C++ code, we add a Vertex structure that uses glm vectors to describe position and color. 
We then create an array of these vertices.
So far so good.

### Vertex Input Binding Descriptions

In order for the device to know how to use our data, we need to provide to it a `VkVertexInputBindingDescription`.
A VertexInput doesn't describe a literal, singular vertex coordinate.
Instead, a VertexInput is something that serves as an input during the vertex STAGE.
This means that anything that is used on a vertex shader basis, probably belongs here.
This structure describes:
- the index of the binding in the array of bindings
- the interval size between elements of the binding 
- Whether Vulkan should take the next stride on a PER INSTANCE basis or on a PER VERTEX basis.

Since we have no instances, the selections should be straight forward.
We create a static function in our new vertex class that will return this structure.

### Vertex Input Attribute Descriptions

So if we have inputs and they can be anything, and it already knows how to make it's stride...how does it know *what* the inputs actually are?
Enter Attribute Descriptions.
For each input, vulkan expects an array of Attribute Descriptions that tell it exactly how to interpret what the hell the chunks of memory are.
Accordingly, Attribute Description structures have the following fields:
- binding: what binding corresponds with the vertex data?
- location: what location parameter in the shader corresponds with this data?
- format: what is the type and size of this data?
	- Common formats: float: VK_FORMAT_R32_SFLOAT, vec2: VK_FORMAT_R32G32_SFLOAT, vec3: VK_FORMAT_R32G32B32_SFLOAT, vec4: VK_FORMAT_R32G32B32A32_SFLOAT
- offset: based off of the beginning from the "strides" it makes, what is the offset where this data starts?

### Informing the Graphics Pipeline

Just because you have input and attributes defined, it doesn't mean that Vulkan knows about your inputs.
Instead, this information is provided when you are creating the graphics pipeline.
You might recall a vertexInputInfo structure, that we gave no data to earlier.
Now we will provide it the information in the form of an array of binding descriptions and an array of attribute descriptions.

Pretty straightforward. Just know that no data has actually been allocated yet. So we are still broken.

## Vertex Buffer Creation

Buffers are the chunks of memory that Vulkan can use to store *stuff*. 
In order to create a buffer, we need ot fill out a `VkBufferCreateInfo` structure.
This structure describes how big a buffer will be, how the buffer will be used (vertex input? index input? memory transfer?), and finally the sharing mode it will have for use between queues.

### Memory Requirements

Before we go and make memory and pretend that everything is dandy, we need to make sure that the kind of memory we are requesting is actually available.
To that end, we use `vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);`.
The structure passed in is populated with with two arrays: memory types, and memory heaps.

The vulkan SDK recomends the following as code for finding an appropriate memory type:
```objectivec
// Find a memory type in "memoryTypeBits" that includes all of "properties"
int32_t FindProperties(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
    for (int32_t i = 0; i < memoryTypeCount; ++i)
    {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    return -1;
}
```

When you have a buffer handle, you can query its requirements with the following:
```objectivec
VkMemoryRequirements memRequirements;
vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);
```

When you have found memory that matches the requirements you need, you can then allocate memory using `VkMemoryAllocateInfo` and a call to `(vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)`.
A big takeaway from this is that **creating buffers does not create memory for that buffer**.

### Memory Association and Mapping

Once you have your memory, you associate it with a particular buffer with `vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);`.
That last parameter is the offset in memory where the buffer memory begins.

Mapping is quite simple. Just remember that the memory needs to have the CPU_Visible property.
If that is the case, writing data to it is simple:
```objectivec
void* data;
vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
vkUnmapMemory(device, vertexBufferMemory);
```
> It is also possible that writes to the buffer are not visible in the mapped memory yet. There are two ways to deal with that problem:
> - Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
> - Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory

Now that the memory has been associated with the buffer, we need to bind the buffer to it once again during rendering.
The following changes were made to the createCommandBuffers function:
```objectivec
VkBuffer vertexBuffers[] = {vertexBuffer};
VkDeviceSize offsets[] = {0};
vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
```

## Staging Buffers

While it is convenient to use shared memory to give data to the gpu, it is not performant to do so. 
Instead, most programs will allocate a second buffer that has memory transfer operations allowed.
This allows us to have a buffer for sending data to the GPU, and another buffer that is GPU only and is faster to use for the GPU.

For copying data over to the Device local memory, we need to create a new command buffer (ideally one from a pool that has the transient bit set).
Using this buffer, we use the CmdCopyBuffer command, and then submit the command buffer to a queue that has a queue transfer bit set for its family.
Now we *could* alter logical device creation slightly to select such a queue, but thankfully, graphics queue families implicitly support transfer operations.
Therefore, we submit our command buffer to the graphics queue.

Of note, we still have to worry about race conditions here, so one might consider using a fence to prevent bad things.
In our case, we went with lazy and waited for the device to become idle.

## Index Buffers

When dealing with vertices, especially strips and or meshes, many of the same vertices are used in several different triangles.
If we used only a vertex input buffer, we would be forced to repeat many of these vertices.
This can quickly end up consuming much more memory than is necessary.
Therefore, we introduce the index buffer.
This buffer holds indices to the vertex input objects.
By binding this buffer in our command buffers, and changing our draw call to a draw indexed call, we can spare ourselves a lot of trouble.

Nothing about this buffers creation is particularly special or different.
Just make sure that the buffer has the index bit set (optionally device local and others depending on needs).
With the bind index buffer command, the parameters are:
command buffer, the index buffer, the offset into memory, and the index size type.

When drawing, you can optionally specify the particular instance, the index to begin at, an offset to the indices(in value), and an offset for instancing.
