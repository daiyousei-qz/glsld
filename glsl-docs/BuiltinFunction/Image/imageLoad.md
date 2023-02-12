Loads the texel at the coordinate P from the image unit image (in IMAGE_PARAMS). For multisample loads, the sample number is given by sample. When image, P, and sample identify a valid texel, the bits used to represent the selected texel in memory are converted to a vec4, ivec4, or uvec4 in the manner described in section 8.26 “Texture Image Loads and Stores” of the OpenGL Specification and returned.