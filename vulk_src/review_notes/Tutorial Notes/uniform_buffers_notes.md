# Uniform Buffers and Descriptors

While rectangles are nice, we would like to be able to actually get some camera action in this bitch.
Since we would like the Model (Transforms done to the model), View (The position, angle, and direction of the camera), and Projection (Orthograthic/Perspective) matrices to be the same for each vertex, it doesn't make sense to copy it over and over for vertex input.
Instead, we take advantage of Uniform Buffers.

## Uniform Buffers

A uniform buffer is one in which all shaders/stages have access to the same data.
This can significantly reduce memory usage if you were to copy it as vertex input.
While not limited to MVP matrices, we will use it for such in our program.

Like vertex input buffers and index buffers, we will need to get some handles on uniform buffers that have been allocated.
This means that it is a buffer that is created with a uniform buffer bit set.
For the memory itself: we are going with HOST VISIBLE, HOST COHERENT bits set.
This is because we will frequently be altering the data in the uniform buffer as our camera changes overtime.

Finally, we create the same amount of these buffers as there are swap images.
This is because we want to preserve the data that was set previously while frames are being worked on in the pipeline.
As the number is directly based on the number of swap images, it is subject to change when the window is recreated.
Therefore, we need to manage the buffers with our swap chain recreation and cleanup.

### Descriptor Layout

While the creation of uniform buffers and memory are quite straight forward, they unfortunately tell the API nothing about how to interpret the allocated memory and how to use it.
Therefore we must provide the API a descriptor layout.
The descriptor layout does what it says on tin: describes the layout of a descriptor.

The first step in creation is at least one `VkDescriptorSetLayoutBinding`.
This describes what index the binding is, what type of descriptor is being described, and how many of these types are associated with the binding.
Other attributes include:
- What stages the descriptor will be used in
- If there are samplers (usable only with image sampling descriptors)

Now that we have at least one binding described (a single Uniform buffer descriptor), we can create a layout.
We need to fill in a creation struct as with literally everything else we create.
This time it's a `VkDescriptorSetLayoutCreateInfo` structure.
It is simple enough: it just takes an array of bindings.

### Informing the pipeline

Great we have a descriptor layout. Big whoop.
What do you do with it?

Well, you need to have it off to the pipeline as it needs to know that about the bindings the shaders will be consuming.
To that end, you inform the pipelineLayout structure by passing it your array of descriptor layouts.
 
### Updating the Uniform Buffers

We will update our uniform buffer in the draw frame function.
We add a function called updateUniformBuffer.
This function is passed the index of the frame that is currently being operated on.

In this function, we construct our MVP matrices using some glm functions.
To put it simply: we are rotating the model 90 degrees every second, our camera is in a fixed position looking down at the origin at a 45 degree angle.
Our perspective is a FOV of 45 degrees and a ratio that is based off of the ratio of our current swap chain extent.

Of note: we set the y coordinate of our project matrix to a negative value since the functions used anticipate openGL versions of screen space.

Lastly, we access the buffer by the map memory function, and memcpy the MVP matrices we just created to it.
Don't forget to unmap the memory.

## Descriptor Sets and Descriptor Pools
 
Descriptor sets are like command buffers in that they cannot be allocated directly.
As such, they need to be allocated from a pool.
Unsurprisingly, we need to create a `VkDescriptorPool`.
However, the descriptor pool needs to know what kind of pools it will be dealing with.
Therefore we need at least one `VkDescriptorPoolSize` structure.

The `VkDescriptorPoolSize` structure describes only two things: 
- The type of descriptor it is associated with
- The number of descriptors of that type to allocate

The pool itself describes several *important* things:
1. The array of `VkDescriptorPoolSize` structures
2. The max amount of descriptor sets to be allowed to allocate

Fantastic, now we have a pool from which to allocate our descriptor sets.
Remember once again that because buffer counts are based on the swap chain, you need to destroy and recreate the resources once the swap chain has been changed.

### Descriptor Set Allocation

Okay we have our pool, now what?
Well we need to create the descriptor set, which means we need to create one for each frame in our swap chain.
`VkDescriptorSetAllocateInfo` contains all the information needed to allocate a series of descriptor sets.

It takes:
- The pool handle from which to allocate descriptor sets
- The count of descriptor sets to be allocated
- An array of descriptor set layouts to be used for allocating that particular descriptor set.
	- In our case it's copies of the same layout

Now we just make a call to `vkAllocateDescriptorSets` with the needed information.
One of the parameters is altered to contain the array of descriptor set handles that are returned.

**Important!**
Descriptor sets start life uninitialized and largely undefined.
Realize that by now, you have never associated a particular uniform buffer to any particular descriptor set.
You only have descriptor sets that said: "Hey I have a Uniform Buffer and it has a binding # of ...".
This means that we need to update the descriptor sets to point to our buffers.

#### Updating and Writing to Descriptor Sets

We begin by specifying a `VkDescriptorBufferInfo` structure.
This structure receives a handle to a buffer, an offset into that buffer, and a range of memory to associate with the descriptor.

Great. Now what?
Well we need to write the information to the descriptor in question.
For that we use `VkWriteDescriptorSet`.
This specifies several important things: 
- The destination descriptor set to be written to
- The particular descriptor binding to write to
- The particular array index to begin writing to (in our case we aren't using an array, so 0)
- The type of the descriptor binding we are writing to
- The amount of descriptors of that binding we are writing to
- The array of structures used to write/update the descriptor
	- bufferInfo: for buffer descriptors
	- imageInfo: for descriptors that reference image information
	- texelBufferView: for descriptors that reference buffer views

Great! With that filled out we only need a call to `vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);`
This can take many write structures (or copy structures) so we pass in the size and pointer.

## Using Descriptor Sets

This is very difficult.
When we are recording our command buffer, we need to bind the descriptor sets that we would like to use.
...It's a call to `vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);`

That's it.
Good job.

Of note for this particular project: with the transform we are applying to the vertexes, we end up reversing the order of vertices that are read in.
As a result, the vertices are considered to be facing away from us.
We only have to change our rasterizer state in our createPipeline function to fix this.

## Alignment Requirements.

> Vulkan expects the data in your structure to be aligned in memory in a specific way, for example:
>
>      Scalars have to be aligned by N (= 4 bytes given 32 bit floats).
>      A vec2 must be aligned by 2N (= 8 bytes)
>      A vec3 or vec4 must be aligned by 4N (= 16 bytes)
>      A nested structure must be aligned by the base alignment of its members rounded up to a multiple of 16.
>      A mat4 matrix must have the same alignment as a vec4.

[The specification](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap14.html#interfaces-resources-layout) has more information on alignment requirements.

Just know that it's better to be explicit than to just assume that everything will be dandy.

#### One Last Thing

The whole point of multiple descriptor sets is that you can have them vary on a per object basis. 
So you could have one object scaling over time while another rotates, etc...
This also means that you can share some of the descriptors across all objects; such as the View and Projection matrices.
