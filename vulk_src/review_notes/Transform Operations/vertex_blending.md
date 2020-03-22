# Vertex Blending

A vertex is transformed based on the transformations of bones and weights.

Needs: w_i where i is the weight of a particular bone for a vertex
M_i is the xform from bone_i to world space coords
B_i is the bones worldd xform that changes wwith timee to animate.

Inn practice: B and M-1 are concatonated.
Normals for surface willl need to be xformed as well using the same matrix.

The vertices are sent only once!
Vertex shader will calcuulate.

The bone mats can be done with UBO
Consider using a BO for M_i since it won't really change.

Use quats for rotations

Better than basic! Use two quats.
Cann lead  to bulging sommetimes.

Relies on rigid body xfforms for use.
