# Vulkan Setup Overview/Review

## What Was Done

1. GLFW was initialized
2. Vulkan **instance** was created
2. An optional **Debug Messenger Object** was created
3. A **physical device** was selected
4. A **logical device** was created

### Vulkan Instance

- An ApplicationInfo structure was created and supplied with information.
- An InstanceCreateInfo structure was created and also supplied with information.
  - This structure was provided with extension count and names (`char**`)
  - This structure was provided with a DebugMessengerInfo structure as well
- Vk_Success is returned if successful. Check for it.
- Remember, created things must also be destroyed in LIFO order

### Debug Messenger Object

- This object is from an extension and therefore `getInstanceProcAddr` needs to be called before you can use the `vkCreateDebug...` function.
- The structure is given info on what types of things to look for, optional user data, and a function to call
  - This function has the signature: ```cpp
  VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT, 
  VkDebugUtilsMessageTypeFlagsEXT,
  const VkDebugUtilsMessengerCallbackDataEXT*,
  void *)```

### Physical Device

- Count vulkan devices by using suitable enumerate function
- Populate an array/vector with `VkPhysicalDevice`
- Find a suitable device by looking at properties and features that are required
	- Suitable devices can also be determined by Queues that are available
- Optionally rank the available devices based on your needs (or maybe allow user to pick)

### Logical Device

- Logical devices are the abstraction that Queues are connected to
- First create an array of QueueCreateInfo structs that determine the family, count, and priority of desired queues.
- Then create a DeviceCreateInfo struct and provide it with:
	- The enabled desired features
	- validation layer info (not really used but good for legacy)
	- The desired queues to be created
- CreateDevice with the info and provide the chosen physical device
- Remember to get a handle to the queues that you created so that they can be used freely
- Again, remember to delete objects if they are created
