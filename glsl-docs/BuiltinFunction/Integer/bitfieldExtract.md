Extracts bits \[offset, offset + bits - 1\] from value, returning them in the least significant bits of the result.

For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit offset + bits - 1.

If bits is zero, the result will be zero. The result will be undefined if offset or bits is negative, or if the sum of offset and bits is greater than the number of bits used to store the operand. Note that for vector versions of bitfieldExtract(), a single pair of offset and bits values is shared for all components.