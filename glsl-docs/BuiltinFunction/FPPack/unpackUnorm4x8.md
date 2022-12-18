First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.

The conversion for unpacked fixed-point value f to floating-point is done as follows:

- unpackUnorm2x16: f / 65535.0
- unpackSnorm2x16: clamp(f / 32767.0, -1, +1)
- unpackUnorm4x8: f / 255.0
- unpackSnorm4x8: clamp(f / 127.0, -1, +1)

The first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.