Splits x into a floating-point significand in the range \[0.5,1.0\], and an integral exponent of two, such that

x = significant · 2^exponent

The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.

If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.

If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.