# Shader Resource Binding

Vulkan organizes bindings in groups, which are called DescriptorSets.
 Each group can itself provide multiple bindings and there can be multiple such groups in parallel using a different set number.

DescriptorSetLayout: This object describes which bindings are in the DescriptorSet for every shader stage. For example, we define at binding 0 a constant buffer used by both vertex and fragment stages, at binding 1 a storage buffer and at 2 an image only for fragment stage. It is the developer’s responsibility to ensure the shaders (SPIR-V) have compatible definitions for the DescriptorSet.

PipelineLayout: As a pipeline (shader and most important rendering state) can have multiple DescriptorSets, this object defines which DescriptorSetLayouts are used with each set binding number. Using the same DescriptorSetLayouts at the same units across pipelines, has some performance benefits, more about that later.

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


A graphics VkPipeline contains the combination of state needed to perform a draw call. Rendering a scene with different shaders, blending modes, vertex layouts, etc, will require a pipeline for each possibility. 
