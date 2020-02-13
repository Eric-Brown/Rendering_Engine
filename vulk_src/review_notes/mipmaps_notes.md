# Mipmap Creation

Mipmaps are precalculated, downscaled  versions of an image.
Each version is a width/2, height/2  version of the previous version.
Mipmaps are used as a form of LOD.
Smaller images increases rendering speed and avoids Moire patterns.

## Image Creation

Vulkan has each of the mip versions stored  in a different mip level  of a vkImage.
0 is the original.
the number of mip levels is specified when the vkimage  is created.
therefore we need to calc the # of mip levels from the dimensions of the given image.
in this project we are  storing our calculations as a class variable.

We are calculating the mip levels as:

`mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;`

The next part of using mipmaps is to let the API know in the structures for the texture creation, view image creation, and image transitioning.

Since we are using the texture itself to create the mipmaps, the base level needs to be optimized as transfer source  and not destination as we had previously.

The next part of generation is using memory bariers for the blit operation as well as the transitions each level will make from src to read only.
In mipmap generation, we loop through each level and use the previous levels values for the current generatio.

When blittting, youu have the  same filters available as you have with the samppler.
We pass Linear to enable thhe interpolation.

Finally, since we are using this filter we need to check if it's supported first.

Lastly, we update our texture sampler to inform it that there are multiple mipmaps available instead of just the one.
Good times.
