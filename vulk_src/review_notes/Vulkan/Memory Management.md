# Memory Management

## High level:

1. GPU Bulk Data

        DEVICE_LOCAL with no HOST_VISIBLE
        Allocate render targets first. 
        Have the lower priority allocations fall back to CPU-side memory if required via HOST_VISIBLE with HOST_COHERENT but without HOST_CACHED.
 
1. CPU-to-GPU Data Flow

        DEVICE_LOCAL and HOST_VISIBLE

1. GPU-to-CPU Data Flow

        HOST_VISIBLE, HOST_COHERENT and HOST_CACHED

### Pooling Allocations

Pool a group of resources like textures and buffers into a single mem alloc.
Ideally around 256.

### Hidden Paging

When you over do GPU memory, DEVICE_LOCAL mem allocs will fail.
Further, if there are several applications which demand GPU memory, the OS can page memory and hide it from you
This is called "hitching".

### Low-Memory GPUs

When getting a memory surplus,  using DEVICE_LOCAL+HOST_VISIBLE for CPU-write cases can bypass the need to schedule an extra copy. 
Better in low level for the same but as an extension to the DEV_LOCAL heap and use for high bandwidth resources.

### Dedicated Allocation

VK_NV_dedicated_allocation extension that allows applications to provide additional explicit information in order to enable additional optimizations. This functionality has been standardized as the VK_KHR_dedicated_allocation extension 
An application can explicitly express the intent to never alias and never sparsely bind a specific resource via the VK_NV_dedicated_allocation extension, which adds a few structures to the resource creation and memory allocation functions. Those together allow device memory to be allocated for a particular buffer or image resource

#### Usage Guidelines
##### Do's

- Use Dedicated Allocations for
    - Render targets for improved GPU performance
    - Very large buffers/images (dozens of MB)
- Use regular allocations and sub-allocations specifically for everything else
- Make sure that the total number of memory allocations is below VkPhysicalDeviceLimits::maxMemoryAllocationCount
   - Ideally stay < 1024 allocations to reduce CPU overhead on Windows 7
- Base your memory allocation strategy on correct interpretation of the results of the various related Vulkan API calls to safe-guard against implementation differences:
    - vkGetBufferMemoryRequirements
    - vkGetImageMemoryRequirements
    - vkGetPhysicalDeviceMemoryProperties
- Explicitly look for the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT when picking a memory type for resources that are intended to be device local.

##### DON'T

Don't use it for everything.
Don't pick "first" for mem allocs that are intended to be dev local.
Don't put excessive amounts of image resources into GPU accessible memory as that will reduce performance severely.

jlkj

Memory is a precious resource, and it can involve several indirect costs by the operating systems. For example some operating systems have a linear cost over the number of allocations for each submission to a Vulkan Queue. Another scenario is that the operating system also handles the paging state of allocations depending on other proceses, we therefore encourage not using too many allocations and organizing them “wisely”.

On devices with dedicated device memory, it is most likely that all resources that are used for many frames are filled through staging buffers. When updating image data we recommend the use of staging buffers, rather than staging images for our hardware. For a small data buffer, updates via the CommandBuffer provide an alternative approach by inlining the data directly.
While OpenGL does allow a bit of aliasing for buffers, by just allocating a big buffer and using offsets, Vulkan allows the same for all resources now. One of the simpler use-cases for this capability is overlapping many different-sized framebuffer images into the same memory allocation when the application knows only one size is active in the frame
A far safer scenario is just re-using memory allocations of pre-allocated chunks. Every time a new resource is required we check if we have available chunks from which we can sub-allocate, or whether we should create a new chunk. Memory can be recycled after a few frames when the GPU is not accessing it anymore. 

For Buffer memory we recommend making use of the offset mechanism the API provides. Just like in OpenGL, Vulkan allows to binding a range of a buffer. The benefit is that we avoid CPU memory costs for lots of tiny buffers, as well as cache misses by using just the same buffer object and varying the offset.

dev local

vertex and index good fit for this
high bandwidth...changes infrequently

host vis

added latency. gpu reading is slow. bus.
good for staging. read once by gpu!
very large data read by gpu...last resort
uncached hazard.

dev local and host vis
special memory
good for resources updated frequently by cpu and read by gpu.
hazard: driver uses as well && uncached

host vis and host cached
system memory
cpu reads and writes cached
gpu access through pcie
good for gpu writing and cpu reading
good for resources read or accessed randomly on cpu

tips:

pool similar resources for tighter packing

keep object resources together to simplify streaming of that objects resources.

suballocation:
don't allocate a new memory block for each resource: use offsets into block
(small limit on max # of allocations)

Allocate big blocks (256 mb) and sub-allocate ranges
(if smaller than 1 Gi, than 1/8 size or so)

allocations are slow: prefer not to allocate or free blocks during gameplay...if necessary use a background thread

you're not alone: leave 20% of dev local memory alone and 33% of device local host vis

transfer queue is designed for efficient transfer
use in parallel with 3d rendering (good for texture streaming)
does not use any compute or graphics hardware
do xfer long before the data is needed on graphics queue
**can be used for defragmenting**
copy resource to new addr async to rendering
when done, update next frames descriptor

when aliasing, if other resource uses your block: treat as uninitialized memory

minimize usage bits when creating resources
