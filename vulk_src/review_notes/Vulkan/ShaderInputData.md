# Shader Memory I/O

## General Shader Inputs

### Basic I/O

Render pass specifies the types of attachments that will be accessed.
The framebuffer specifies the actual image views to bind to render pass attachments.

For program linkage:

```glsl
//Vert
layout(location = 0) out vec4 color;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec3 normal;
```

Corresponds with:

```glsl
//Frag
layout(location = 0) in vec4 diffuseAlbedo;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 cameraSpaceNormal;
```

*Remember: Some data types take up multiple locations!!!*

When doing block declarations, locations are auto generated based on size and starting location.

### Managing auto allocated sizes

GLSL assumes everything is passed around as vec4 components.
Therefore: `mat2` will take up 2 locations.
However, you can reclaim space by using the same location with a different component.

```glsl
layout(location = 0) out vec2 arr1[5];
layout(location = 0, component = 2) out vec2 arr2[4]; //Different sizes are fine.
layout(location = 4, component = 2) out float val;    //A non-array takes the last two fields from location 4.
```

### Binding Points

Buffer backed interface blocks and opaque types(images, samplers, atomic counters) have a setting which represents an index in the current context.

```glsl
layout(binding = 3) uniform sampler2D mainTexture;
layout(binding = 1, std140) uniform MainBlock
{
  vec3 data;
};
```

The `descriptor layout` specifies the types of resources that are going to be accessed by the pipeline.
A `descriptor set` specifies the actual buffer or image resources that will be bound to the descriptors.

If array of UBOs(like array of bones), layout would have a higher count.

## Vertex Shader

```cpp
	vk::PipelineVertexInputStateCreateInfo...(..., &bindingDescription1,...,attributeDescriptions.data());
```

BindingDescription describes the Vertex object (i.e. is it an instance? whats the size?)

VertexInputAttributeDescription describes the sub parts of the object.
It corresponds with `layout(location=#) **in** type name;`

More specifically:

```glsl
layout(location = 2) in vec3 values[4];
//This will allocate the attribute indices 2, 3, 4, and 5.
```

## Frag Shader

Specify the index of the buffer to write to:

```glsl
layout(location = output index) out vec4 outColor;
// For dual source blending
layout(location = output index, index = dual output index) out vec4 outColor;
```

I think this is coordinated with the attachments when creating the render pass?


