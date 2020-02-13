# Mipmap Creation

Mipmapps are precalculated, downscaled  versions of an image.
Each version is a width/2, heeight/2  version of the previos version.
Mipmaps are used as a form of LOD.
Smaller images increases rendering speed and avoids Moire patterns.

## Image Creation

Vulkan has each of the mip versions stored  in a different mip level  of a vkImage.
0 is the original.
the nuumber of mip levels is sppecified when the vkimage  is created.
therefore we need to caalc the # of mip levels from the dimensions of the given image.
in this project we are  storing our calculationns as a class variable.
