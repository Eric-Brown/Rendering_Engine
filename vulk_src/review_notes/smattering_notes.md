# A  Smattering of Notes

## Push Constants and Dynamic Uniform Memory

All the guidelines out there are pretty clear when they say to use push-constants for data that has to change on a per-object basis every frame, 

mapping memory is a really slow process, so if you need something mapped, keep it that way for as long as you can

size_t deviceAlignment = deviceProps.limits.minUniformBufferOffsetAlignment;
Once you know the alignment you need, you can use Windows’ aligned_malloc function to actually get an aligned block of memory, which you can then memcpy into the vkbuffer’s mapped pointer.
Likewise, when you allocate your VkBuffer, you’re going to want to request a buffer of size dynamicAlignment * number of primitives, and you’ll want to make sure you get memory that comes from a descriptorPool of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.

layout(push_constant) uniform PER_OBJECT
{
    mat4 mvp;
    vec4 col;
} obj;

Next, instead of creating any VkBuffers, when we create our pipeline layout, we need to specify a push constant range:

## Pipeline Cache

Pipeline cache objects allow the result of pipeline construction to be reused between pipelines and between runs of an application. 
    Reuse between pipelines is achieved by passing the same pipeline cache object when creating multiple related pipelines.
    Reuse across runs of an application is achieved by retrieving pipeline cache contents in one run of an application, saving the contents, and using them to preinitialize a pipeline cache on a subsequent run
    
## Binding stuff

```
// example for typical loops in rendering
for each view {
  bind view resources          // camera, environment...
  for each shader {
    bind shader pipeline  
    bind shader resources      // shader control values
    for each material {
      bind material resources  // material parameters and textures
      for each object {
        bind object resources  // object transforms
        draw object
      }
    }
  }
}
```

By making proper use of the parallel DescriptorSet bindings and PipelineLayouts the software developers can now represent this in Vulkan (increasing set number as we descend). In principle you can do this in previous APIs as well, however, Vulkan tells the driver up front that in this example the “view” bindings, would be common to all shaders at the same binding slot. A traditional API would have to inspect all the software bindings when the shaders are changed with less apriori knowledge about which are being overwritten and which are important to keep.

We recommend making use of the different set numbers, to avoid redundant bindings. Putting many bindings that have very different frequencies in the same DescriptorSet can be bad for overall performance. Imagine a DescriptorSet with several textures and uniform buffer binding of which only one changes, that’s potentially a lot of data being sent to the GPU that effectively doesn’t do anything.

Vulkan provides different approaches for this as well. In principle uniform data can be fed in three ways:

    Uniform Buffer Binding: As part of a DescriptorSet this would be the equivalent of an arbitrary glBindBufferRange(GL_UNIFORM_BUFFER, dset.binding, dset.bufferOffset, dset.bufferSize) in OpenGL. All information for the actual binding by the CommandBuffer is stored within the DescriptorSet itself.
    Uniform Buffer Dynamic Binding: Similar as above, but with the ability to provide the bufferOffset later when recording the CommandBuffer, a bit like this pseudo code: CommandBuffer->BindDescriptorSet(setNumber, descriptorSet, &offset). It is very practical to use when sub-allocating uniform buffers from a larger buffer allocation.
    Push Constants: PushConstants are uniform values that are stored within the CommandBuffer and can be accessed from the shaders similar to a single global uniform buffer. They provide enough bytes to hold some matrices or index values and the interpretation of the raw data is up the shader. You may recall glProgramEnvParameter from OpenGL providing something similar. The values are recorded with the CommandBuffer and cannot be altered afterwards: CommandBuffer->PushConstant(offset, size, &data)
Re-using the same DescriptorSet with just different offsets is rather CPU-cache friendly as well compared to using and managing many DescriptorSets. 
