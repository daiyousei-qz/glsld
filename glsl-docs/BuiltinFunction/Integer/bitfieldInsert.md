Inserts the bits least significant bits of insert into base.

The result will have bits [offset, offset + bits - 1] taken from bits [0, bits - 1] of insert, and all other bits taken directly from the corresponding bits of base. If bits is zero, the result will simply be base. The result will be undefined if offset or bits is negative, or if the sum of offset and bits is greater than the number of bits used to store the operand.
Note that for vector versions of bitfieldInsert(), a single pair of offset and bits values is shared for all components.