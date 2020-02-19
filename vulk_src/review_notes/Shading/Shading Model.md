# Shading Models

1st step in determining apppearance isto choose a shading model to describe howw the objects color should vary based on factors.

Shading models often hhave properties used to  control appearance variation.

mmost shading models use:
n - normal vector of surface
l - light direction vector
v - view directon vector.

dot product between normalized vectors will give quick n dirty cosine value

linear interpolation  is so frequennt that it is usually built in:
`mix` or `lerp`

reflection is done so frequently it also is frequently built in

## Gooch Shading

Designed to increase legibility of details in tech lllustrations.
The basic idea behind Gooch shading is to compare the surface normal to the light’s location.
If the normal points toward the light, a warmer tone is used to color the surface; if it points away, a cooler tone is used.
Angles in-between are interpolated.

cshaded = s chighlight + (1 − s) t cwarm + (1 − t) ccool)

t = (n * l)+1/2

r = 2(n*l)n-1 // reflected light vector

s = (100(r*v) - 97) ///clamped (0,1)

