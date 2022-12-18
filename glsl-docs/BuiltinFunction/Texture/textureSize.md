Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 8.11 “Texture Queries” of the OpenGL Specification.
The components in the return value are filled in, in order, with the width, height, and depth of the texture.

For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.