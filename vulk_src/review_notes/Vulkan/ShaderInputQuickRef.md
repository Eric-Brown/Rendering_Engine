# Shader Input Quick Reference

## How Do I Questions

    How do I add a new uniform buffer as shader input?

`DescriptorSetLayout` needs to be updated with the information on the new binding. **Remember that if you have multiple pipelines, the layout needs to be correct for each one.**

`DescriptorPool` needs to be updated with the information on the new binding. Give it a new `DescriptorPoolSize`. **Ask...how many do I need?**

`DescriptorSet` needs to be allocated with the new layout.

`DescriptorSet` needs to be configured with `DescriptorBufferInfo` and written into the descriptor set. **Ask...is this data part of the same descriptor set?**

`BindDescriptorSets` will then allow you to access the buffer in the shader.

    How do I add a new vertex attribute?

Easy! 