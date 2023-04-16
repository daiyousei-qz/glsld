First, converts each component of the normalized floating-point value v into 16-bit (2x16) or 8-bit (4x8) integer values. Then, the results are packed into the returned 32-bit unsigned integer.

The conversion for component c of v to fixed point is done as follows:

- packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)
- packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)
- packUnorm4x8: round(clamp(c, 0, +1) * 255.0)
- packSnorm4x8: round(clamp(c, -1, +1) * 127.0)

The first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.