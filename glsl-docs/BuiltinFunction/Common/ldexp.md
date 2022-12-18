Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:

significand · 2^exponent

If this product is too large to be represented in the floating-point type, the result is undefined.

If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-denormalized values.
If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.