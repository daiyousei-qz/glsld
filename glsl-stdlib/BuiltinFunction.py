from dataclasses import dataclass
from re import match
from typing import Generator, TypedDict
from StdlibBuilder import StdlibBuilder, GenTypePrefix, VectorGenTypeInfo, allMatrixGenTypes, allVectorGenTypes

GEN_FTYPE_PREFIXES: list[GenTypePrefix] = ["", "f16"]
GEN_DTYPE_PREFIXES: list[GenTypePrefix] = ["d"]
GEN_ITYPE_PREFIXES: list[GenTypePrefix] = ["i", "i8", "i16", "i64"]
GEN_UTYPE_PREFIXES: list[GenTypePrefix] = ["u", "u8", "u16", "u64"]
GEN_BTYPE_PREFIXES: list[GenTypePrefix] = ["b"]

@dataclass
class TextureConfig:
    has1D: bool = False
    has2D: bool = False
    has3D: bool = False
    hasCube: bool = False
    hasArray: bool = False
    hasShadow: bool = False
    hasRect: bool = False
    hasBuffer: bool = False
    hasMS: bool = False

    def test(self, filter: 'TextureConfig') -> bool:
        if not filter.has1D and self.has1D:
            return False
        if not filter.has2D and self.has2D:
            return False
        if not filter.has3D and self.has3D:
            return False
        if not filter.hasCube and self.hasCube:
            return False
        if not filter.hasArray and self.hasArray:
            return False
        if not filter.hasShadow and self.hasShadow:
            return False
        if not filter.hasRect and self.hasRect:
            return False
        if not filter.hasBuffer and self.hasBuffer:
            return False
        if not filter.hasMS and self.hasMS:
            return False
        return True

def getAllTextureModifiers():
    def makeResult(prefix, postfix):
        setup = TextureConfig()
        dim = 1

        if "1D" in postfix:
            setup.has1D = True
            dim = 1
        if "2D" in postfix:
            setup.has2D = True
            dim = 2
        if "3D" in postfix:
            setup.has3D = True
            dim = 3
        if "Cube" in postfix:
            setup.hasCube = True
            dim = 3
        if "Array" in postfix:
            setup.hasArray = True
        if "Shadow" in postfix:
            setup.hasShadow = True
        if "Rect" in postfix:
            setup.hasRect = True
        if "Buffer" in postfix:
            setup.hasBuffer = True
        if "MS" in postfix:
            setup.hasMS = True

        return (prefix, postfix, dim, setup)

    allPrefixes: list[GenTypePrefix] = ["", "i", "u"]
    for prefix in allPrefixes:
        for postfix in ["1D", "2D", "Cube"]:
            yield makeResult(prefix, postfix)
            yield makeResult(prefix, f"{postfix}Array")
            if prefix == "":
                yield makeResult(prefix, f"{postfix}Shadow")
                yield makeResult(prefix, f"{postfix}ArrayShadow")

        yield makeResult(prefix, "3D")
        yield makeResult(prefix, "2DRect")
        if prefix == "":
            yield makeResult(prefix, "2DRectShadow")
        yield makeResult(prefix, "Buffer")
        yield makeResult(prefix, "2DMS")
        yield makeResult(prefix, "2DMSArray")

# 8.1 Angle and Trigonometry Functions
def addTrigonometryFunctions(builder: StdlibBuilder):
    documentations = {
        "radians": """
Converts degrees to radians, i.e. π / 180 * degrees.
""",
        "degrees": """
Converts radians to degrees, i.e. 180 / π * radians.
""",
        "sin": """
The standard trigonometric sine function.
""",
        "cos": """
The standard trigonometric cosine function.
""",
        "tan": """
The standard trigonometric tangent function.
""",
        "asin": """
Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
""",
        "acos": """
Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
""",
        "atan": """
Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
""",
        "sinh": """
Returns the hyperbolic sine function (e^x - e^-x) / 2.
""",
        "cosh": """
Returns the hyperbolic cosine function (e^x + e^-x) / 2.
""",
        "tanh": """
Returns the hyperbolic tangent function sinh(x) / cosh(x).
""",
        "asinh": """
Arc hyperbolic sine; returns the inverse of sinh.
""",
        "acosh": """
Arc hyperbolic cosine; returns the inverse of cosh.
""",
        "atanh": """
Arc hyperbolic tangent; returns the inverse of tanh.
""",
    }

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES):
        builder.addFunction(f"{genType} radians({genType} degrees);", documentations["radians"], genType.guardMacros)
        builder.addFunction(f"{genType} degrees({genType} radians);", documentations["degrees"], genType.guardMacros)
        builder.addFunction(f"{genType} sin({genType} x);", documentations["sin"], genType.guardMacros)
        builder.addFunction(f"{genType} cos({genType} x);", documentations["cos"], genType.guardMacros)
        builder.addFunction(f"{genType} tan({genType} x);", documentations["tan"], genType.guardMacros)
        builder.addFunction(f"{genType} asin({genType} x);", documentations["asin"], genType.guardMacros)
        builder.addFunction(f"{genType} acos({genType} x);", documentations["acos"], genType.guardMacros)
        builder.addFunction(f"{genType} atan({genType} x);", documentations["atan"], genType.guardMacros)
        builder.addFunction(f"{genType} sinh({genType} x);", documentations["sinh"], genType.guardMacros)
        builder.addFunction(f"{genType} cosh({genType} x);", documentations["cosh"], genType.guardMacros)
        builder.addFunction(f"{genType} tanh({genType} x);", documentations["tanh"], genType.guardMacros)
        builder.addFunction(f"{genType} asinh({genType} x);", documentations["asinh"], genType.guardMacros)
        builder.addFunction(f"{genType} acosh({genType} x);", documentations["acosh"], genType.guardMacros)
        builder.addFunction(f"{genType} atanh({genType} x);", documentations["atanh"], genType.guardMacros)

# 8.2 Exponential Functions
def addExponentialFunctions(builder: StdlibBuilder):
    documentations = {
        "pow": """
Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
""",
        "exp": """
Returns the natural exponentiation of x, i.e., e^x.
""",
        "log": """
Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
""",
        "exp2": """
Returns 2 raised to the x power, i.e., 2^x.
""",
        "log2": """
Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
""",
        "sqrt": """
Returns sqrt(x). Results are undefined if x < 0.
""",
        "inversesqrt": """
Returns 1 / sqrt(x). Results are undefined if x <= 0.
"""
    }

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES):
        builder.addFunction(f"{genType} pow({genType} x, {genType} y);", documentations["pow"], genType.guardMacros)
        builder.addFunction(f"{genType} exp({genType} x);", documentations["exp"], genType.guardMacros)
        builder.addFunction(f"{genType} log({genType} x);", documentations["log"], genType.guardMacros)
        builder.addFunction(f"{genType} exp2({genType} x);", documentations["exp2"], genType.guardMacros)
        builder.addFunction(f"{genType} log2({genType} x);", documentations["log2"], genType.guardMacros)
        builder.addFunction(f"{genType} sqrt({genType} x);", documentations["sqrt"], genType.guardMacros)
        builder.addFunction(f"{genType} inversesqrt({genType} x);", documentations["inversesqrt"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{genType} sqrt({genType} x);", documentations["sqrt"], genType.guardMacros)
        builder.addFunction(f"{genType} inversesqrt({genType} x);", documentations["inversesqrt"], genType.guardMacros)

# 8.3 Common Functions
def addCommonFunctions(builder: StdlibBuilder):
    documentations = {
        "abs": """
Returns x if x >= 0; otherwise it returns -x.
""",
        "sign": """
Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
""",
        "floor": """
Returns a value equal to the nearest integer that is less than or equal to x.
""",
        "trunc": """
Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
""",
        "round": """
Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
""",
        "roundEven": """
Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
""",
        "ceil": """
Returns a value equal to the nearest integer that is greater than or equal to x.
""",
        "fract": """
Returns x - floor(x).
""",
        "mod": """
Modulus. Returns x - y · floor(x / y).

Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
""",
        "modf": """
Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
""",
        "min": """
Returns y if y < x; otherwise it returns x.
""",
        "max": """
Returns y if x < y; otherwise it returns x.
""",
        "clamp": """
Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
""",
        "mix_linear": """
Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
""",
        "mix_select": """
Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
""",
        "step": """
Returns 0.0 if x < edge; otherwise it returns 1.0.
""",
        "smoothstep": """
Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:

```
genFType t;
t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
return t * t * (3 - 2 * t);
```

(And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
""",
        "isnan": """
Returns true if x holds a NaN. Returns false otherwise.
""",
        "isinf": """
Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
""",
        "floatBitsToInt": """
Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
""",
        "floatBitsToUint": """
Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
""",
        "intBitsToFloat": """
Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
""",
        "uintBitsToFloat": """
Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
""",
        "fma": """
Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:

- fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
- The precision of fma() can differ from the precision of the expression a * b + c.
- fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.

Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
""",
        "frexp": """
Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that

x = significant · 2exponent

The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.

If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.

If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
""",
        "ldexp": """
Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:

significand · 2exponent

If this product is too large to be represented in the floating-point type, the result is undefined.

If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.

If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
""",
    }

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES + GEN_ITYPE_PREFIXES):
        builder.addFunction(f"{genType} abs({genType} x);", documentations["abs"], genType.guardMacros)
        builder.addFunction(f"{genType} sign({genType} x);", documentations["sign"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{genType} floor({genType} x);", documentations["floor"], genType.guardMacros)
        builder.addFunction(f"{genType} trunc({genType} x);", documentations["trunc"], genType.guardMacros)
        builder.addFunction(f"{genType} round({genType} x);", documentations["round"], genType.guardMacros)
        builder.addFunction(f"{genType} roundEven({genType} x);", documentations["roundEven"], genType.guardMacros)
        builder.addFunction(f"{genType} ceil({genType} x);", documentations["ceil"], genType.guardMacros)
        builder.addFunction(f"{genType} fract({genType} x);", documentations["fract"], genType.guardMacros)
        builder.addFunction(f"{genType} mod({genType} x, {genType.scalarType} y);", documentations["mod"], genType.guardMacros)
        builder.addFunction(f"{genType} mod({genType} x, {genType} y);", documentations["mod"], genType.guardMacros)
        builder.addFunction(f"{genType} modf({genType} x, out {genType} i);", documentations["modf"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES + GEN_ITYPE_PREFIXES + GEN_UTYPE_PREFIXES):
        builder.addFunction(f"{genType} min({genType} x, {genType} y);", documentations["min"], genType.guardMacros)
        builder.addFunction(f"{genType} min({genType} x, {genType.scalarType} y);", documentations["min"], genType.guardMacros)
        builder.addFunction(f"{genType} max({genType} x, {genType} y);", documentations["max"], genType.guardMacros)
        builder.addFunction(f"{genType} max({genType} x, {genType.scalarType} y);", documentations["max"], genType.guardMacros)
        builder.addFunction(f"{genType} clamp({genType} x, {genType} minVal, {genType} maxVal);", documentations["clamp"], genType.guardMacros)
        builder.addFunction(f"{genType} clamp({genType} x, {genType.scalarType} minVal, {genType.scalarType} maxVal);", documentations["clamp"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{genType} mix({genType} x, {genType} y, {genType} a);", documentations["mix_linear"], genType.guardMacros)
        builder.addFunction(f"{genType} mix({genType} x, {genType} y, {genType.scalarType} a);", documentations["mix_linear"], genType.guardMacros)
        builder.addFunction(f"{genType} step({genType} edge, {genType} x);", documentations["step"], genType.guardMacros)
        builder.addFunction(f"{genType} step({genType.scalarType} edge, {genType} x);", documentations["step"], genType.guardMacros)
        builder.addFunction(f"{genType} smoothstep({genType} edge0, {genType} edge1, {genType} x);", documentations["smoothstep"], genType.guardMacros)
        builder.addFunction(f"{genType} smoothstep({genType.scalarType} edge0, {genType.scalarType} edge1, {genType} x);", documentations["smoothstep"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES + GEN_ITYPE_PREFIXES + GEN_UTYPE_PREFIXES + GEN_BTYPE_PREFIXES):
        builder.addFunction(f"{genType} mix({genType} x, {genType.getBType()} y, {genType.getBType()} a);", documentations["mix_select"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{genType.getBType()} isnan({genType} x);", documentations["isnan"], genType.guardMacros)
        builder.addFunction(f"{genType.getBType()} isinf({genType} x);", documentations["isinf"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES):
        ivec = genType.getITypeForBitcast()
        uvec = genType.getUTypeForBitcast()
        builder.addFunction(f"{ivec} floatBitsToInt(highp {genType} value);", documentations["floatBitsToInt"], genType.guardMacros)
        builder.addFunction(f"{uvec} floatBitsToUint(highp {genType} value);", documentations["floatBitsToUint"], genType.guardMacros)
        builder.addFunction(f"{genType} intBitsToFloat(highp {ivec} value);", documentations["intBitsToFloat"], genType.guardMacros)
        builder.addFunction(f"{genType} uintBitsToFloat(highp {uvec} value);", documentations["uintBitsToFloat"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{genType} fma({genType} a, {genType} b, {genType} c);", documentations["fma"], genType.guardMacros)
        builder.addFunction(f"{genType} frexp(highp {genType} x, out highp {genType.getIType()} exp);", documentations["frexp"], genType.guardMacros)
        builder.addFunction(f"{genType} ldexp(highp {genType} x, highp {genType.getIType()} exp);", documentations["ldexp"], genType.guardMacros)

# 8.4 Floating-Point Pack and Unpack Functions
def addFPPackUnpackFunctions(builder: StdlibBuilder):
    documentations = {
        "pack": """
First, converts each component of the normalized floating-point value *v* into 16-bit (**2x16**) or 8-bit (**4x8**) integer values. Then, the results are packed into the returned 32-bit unsigned integer.

The conversion for component *c* of *v* to fixed point is done as follows:
- **packUnorm2x16**: `round(clamp(c, 0, +1) * 65535.0)`
- **packSnorm2x16**: `round(clamp(c, -1, +1) * 32767.0)`
- **packUnorm4x8**: `round(clamp(c, 0, +1) * 255.0)`
- **packSnorm4x8**: `round(clamp(c, -1, +1) * 127.0)`

The first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.
""",
        "unpack": """
First, unpacks a single 32-bit unsigned integer *p* into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.

The conversion for unpacked fixed-point value *f* to floating-point is done as follows:
- **unpackUnorm2x16**: `f / 65535.0`
- **unpackSnorm2x16**: `clamp(f / 32767.0, -1, +1)`
- **unpackUnorm4x8**: `f / 255.0`
- **unpackSnorm4x8**: `clamp(f / 127.0, -1, +1)`

The first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.
""",
        "packHalf": """
Returns an unsigned integer obtained by converting the components of a two-component floating-point vector to the 16-bit floating-point representation of the API, and then packing these two 16-bit integers into a 32-bit unsigned integer.

The first vector component specifies the 16 least-significant bits of the result; the second component specifies the 16 most-significant bits.
""",
        "unpackHalf": """
Returns a two-component floating-point vector with components obtained by unpacking a 32-bit unsigned integer into a pair of 16-bit values, interpreting those values as 16-bit floating-point numbers according to the API, and converting them to 32-bit floating-point values.

The first component of the vector is obtained from the 16 least-significant bits of *v*; the second component is obtained from the 16 most-significant bits of *v*.
""",
        "packDouble": """
Returns a double-precision value obtained by packing the components of *v* into a 64-bit value. If an IEEE 754 Inf or NaN is created, it will not signal, and the resulting floating-point value is unspecified. Otherwise, the bit-level representation of *v* is preserved. The first vector component specifies the 32 least significant bits; the second component specifies the 32 most significant bits.
""",
        "unpackDouble": """
Returns a two-component unsigned integer vector representation of *v*. The bit-level representation of *v* is preserved. The first component of the vector contains the 32 least significant bits of the double; the second component consists of the 32 most significant bits.
"""
    }

    builder.addFunction("highp uint packUnorm2x16(vec2 v);", documentations["pack"])
    builder.addFunction("highp uint packSnorm2x16(vec2 v);", documentations["pack"])
    builder.addFunction("uint packUnorm4x8(vec4 v);", documentations["pack"])
    builder.addFunction("uint packSnorm4x8(vec4 v);", documentations["pack"])

    builder.addFunction("vec2 unpackUnorm2x16(highp uint p);", documentations["unpack"])
    builder.addFunction("vec2 unpackSnorm2x16(highp uint p);", documentations["unpack"])
    builder.addFunction("vec4 unpackUnorm4x8(highp uint p);", documentations["unpack"])
    builder.addFunction("vec4 unpackSnorm4x8(highp uint p);", documentations["unpack"])

    builder.addFunction("uint packHalf2x16(vec2 v);", documentations["packHalf"])
    builder.addFunction("vec2 unpackHalf2x16(uint v);", documentations["unpackHalf"])

    builder.addFunction("double packDouble2x32(uvec2 v);", documentations["packDouble"])
    builder.addFunction("uvec2 unpackDouble2x32(double v);", documentations["unpackDouble"])

# 8.5 Geometric Functions
def addGeometricFunctions(builder: StdlibBuilder):
    documentations = {
        "length": """
Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
""",
        "distance": """
Returns the distance between p0 and p1, i.e., length(p0 - p1).
""",
        "dot": """
Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
""",
        "cross": """
Returns the cross product of x and y, i.e., (x[1] · y[2] - y[1] · x[2], x[2] · y[0] - y[2] · x[0], x[0] · y[1] - y[0] · x[1]).
""",
        "normalize": """
Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
""",
        "ftransform": """
Available only when using the compatibility profile. For core OpenGL, use invariant.

For vertex shaders only. This function will ensure that the incoming vertex value will be transformed in a way that produces exactly the same result as would be produced by OpenGL's fixed functionality transform. It is intended to be used to compute gl_Position, e.g.

`gl_Position = ftransform()`

This function should be used, for example, when an application is rendering the same geometry in separate passes, and one pass uses the fixed functionality path to render and another pass uses programmable shaders.
""",
        "faceforward": """
If dot(Nref, I) < 0 return N, otherwise return -N.
""",
        "reflect": """
For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
""",
        "refract": """
For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.

The result is computed by the refraction equation.

The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
""",
    }

    for vecType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{vecType.scalarType} length({vecType} x);", documentations["length"], vecType.guardMacros)
        builder.addFunction(f"{vecType.scalarType} distance({vecType} p0, {vecType} p1);", documentations["distance"], vecType.guardMacros)
        builder.addFunction(f"{vecType.scalarType} dot({vecType} x, {vecType} y);", documentations["dot"], vecType.guardMacros)
        builder.addFunction(f"{vecType} normalize({vecType} x);", documentations["normalize"], vecType.guardMacros)
        builder.addFunction(f"{vecType} faceforward({vecType} N, {vecType} I, {vecType} Nref);", documentations["faceforward"], vecType.guardMacros)
        builder.addFunction(f"{vecType} reflect({vecType} I, {vecType} N);", documentations["reflect"], vecType.guardMacros)
        builder.addFunction(f"{vecType} refract({vecType} I, {vecType} N, {vecType.scalarType} eta);", documentations["refract"], vecType.guardMacros)

    builder.addFunction("vec3 cross(vec3 x, vec3 y);", documentations["cross"])
    builder.addFunction("dvec3 cross(dvec3 x, dvec3 y);", documentations["cross"])
    builder.addFunction("vec4 ftransform();", documentations["ftransform"])

# 8.6 Matrix Functions
def addMatrixFunctions(builder: StdlibBuilder):
    documentations = {
        "matrixCompMult": """
Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].

Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
""",
        "outerProduct": """
Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
""",
        "transpose": """
Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
""",
        "determinant": """
Returns the determinant of *m*.
""",
        "inverse": """
Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
"""
    }

    for matType in allMatrixGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES):
        builder.addFunction(f"{matType} matrixCompMult({matType} x, {matType} y);", documentations["matrixCompMult"])
        builder.addFunction(f"{matType} outerProduct({matType.getColumnVectorType()} c, {matType.getRowVectorType()} r);", documentations["outerProduct"])
        builder.addFunction(f"{matType.getTransposedMatrixType()} transpose({matType} m);", documentations["transpose"])
        if matType.dimX == matType.dimY:
            builder.addFunction(f"{matType.scalarType} determinant({matType} m);", documentations["determinant"])
            builder.addFunction(f"{matType} inverse({matType} m);", documentations["inverse"])

# 8.7 Vector Relational Functions
def addVectorRelationalFunctions(builder: StdlibBuilder):
    documentations = {
        "lessThan": """
Returns the component-wise compare of x < y.
""",
        "lessThanEqual": """
Returns the component-wise compare of x <= y.
""",
        "greaterThan": """
Returns the component-wise compare of x > y.
""",
        "greaterThanEqual": """
Returns the component-wise compare of x >= y.
""",
        "equal": """
Returns the component-wise compare of x == y.
""",
        "notEqual": """
Returns the component-wise compare of x != y.
""",
        "any": """
Returns true if any component of x is true.
""",
        "all": """
Returns true if all components of x are true.
""",
        "not": """
Returns the component-wise logical complement of x.
"""
    }

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES + GEN_ITYPE_PREFIXES + GEN_UTYPE_PREFIXES):
        if genType.dim > 1:
            builder.addFunction(f"{genType.getBType()} lessThan({genType} x, {genType} y);", documentations["lessThan"], genType.guardMacros)
            builder.addFunction(f"{genType.getBType()} lessThanEqual({genType} x, {genType} y);", documentations["lessThanEqual"], genType.guardMacros)
            builder.addFunction(f"{genType.getBType()} greaterThan({genType} x, {genType} y);", documentations["greaterThan"], genType.guardMacros)
            builder.addFunction(f"{genType.getBType()} greaterThanEqual({genType} x, {genType} y);", documentations["greaterThanEqual"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES + GEN_DTYPE_PREFIXES + GEN_ITYPE_PREFIXES + GEN_UTYPE_PREFIXES + GEN_BTYPE_PREFIXES):
        if genType.dim > 1:
            builder.addFunction(f"{genType.getBType()} equal({genType} x, {genType} y);", documentations["equal"], genType.guardMacros)
            builder.addFunction(f"{genType.getBType()} notEqual({genType} x, {genType} y);", documentations["notEqual"], genType.guardMacros)

    for genType in allVectorGenTypes(GEN_BTYPE_PREFIXES):
        if genType.dim > 1:
            builder.addFunction(f"bool any({genType} x);", documentations["any"], genType.guardMacros)
            builder.addFunction(f"bool all({genType} x);", documentations["all"], genType.guardMacros)
            builder.addFunction(f"{genType} not({genType} x);", documentations["not"], genType.guardMacros)

# 8.8 Integer Functions
def addIntegerFunctions(builder: StdlibBuilder):
    documentations = {
        "uaddCarry": """
Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
""",
        "usubBorrow": """
Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
""",
        "mulExtended": """
Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
""",
        "bitfieldExtract": """
Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.

For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.

If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
""",
        "bitfieldInsert": """
Inserts the *bits* least significant bits of *insert* into *base*.

The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
""",
        "bitfieldReverse": """
Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
""",
        "bitCount": """
Returns the number of one bits in the binary representation of *value*.
""",
        "findLSB": """
Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
""",
        "findMSB": """
Returns the bit number of the most significant bit in the binary representation of *value*.

For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
"""
    }

    for genType in allVectorGenTypes(GEN_UTYPE_PREFIXES):
        builder.addFunction(f"{genType} uaddCarry(highp {genType} x, highp {genType} y, out lowp {genType} carry);", documentations["uaddCarry"])
        builder.addFunction(f"{genType} usubBorrow(highp {genType} x, highp {genType} y, out lowp {genType} borrow);", documentations["usubBorrow"])
        builder.addFunction(f"void umulExtended(highp {genType} x, highp {genType} y, out highp {genType} msb, out highp {genType} lsb);", documentations["mulExtended"])

    for genType in allVectorGenTypes(GEN_ITYPE_PREFIXES):
        builder.addFunction(f"void imulExtended(highp {genType} x, highp {genType} y, out highp {genType} msb, out highp {genType} lsb);", documentations["mulExtended"])

    for genType in allVectorGenTypes(GEN_ITYPE_PREFIXES + GEN_UTYPE_PREFIXES):
        builder.addFunction(f"{genType} bitfieldExtract({genType} value, int offset, int bits);", documentations["bitfieldExtract"])
        builder.addFunction(f"{genType} bitfieldInsert({genType} base, {genType} insert, int offset, int bits);", documentations["bitfieldInsert"])
        builder.addFunction(f"{genType} bitfieldReverse(highp {genType} value);", documentations["bitfieldReverse"])
        builder.addFunction(f"{genType.getIType()} bitCount({genType} value);", documentations["bitCount"])
        builder.addFunction(f"{genType.getIType()} findLSB({genType} value);", documentations["findLSB"])
        builder.addFunction(f"{genType.getIType()} findMSB(highp {genType} value);", documentations["findMSB"])

# 8.9 Texture Functions
def addTextureFunctions(builder: StdlibBuilder):
    documentations = {
        "textureSize": """
Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
The components in the return value are filled in, in order, with the width, height, and depth of the texture.

For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
""",
        "textureQueryLod": """
Returns the mipmap array(s) that would be accessed in the x component of the return value.

Returns the computed level-of-detail relative to the base level in the y component of the return value.

If called on an incomplete texture, the results are undefined.
""",
        "textureQueryLevels": """
Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.

The value zero will be returned if no texture or an incomplete texture is associated with sampler.

Available in all shader stages.
""",
        "textureSamples": """
Returns the number of samples of the texture or textures bound to sampler.
""",
        "texture": """
Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.

For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)

For non-shadow forms: the array layer comes from the last component of P.
""",
        "textureProj": """
Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
""",
        "textureLod": """
Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
(See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)

∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
""",
        "textureOffset": """
Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.

Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
Note that texel offsets are also not supported for cube maps.
""",
        "texelFetch": """
Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
""",
        "texelFetchOffset": """
Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
""",
        "textureProjOffset": """
Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
""",
        "textureLodOffset": """
Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
""",
        "textureProjLod": """
Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
""",
        "textureProjLodOffset": """
Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
""",
        "textureGrad": """
Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
""",
        "textureGradOffset": """
Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
""",
        "textureProjGrad": """
Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
""",
        "textureProjGradOffset": """
Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
""",
        "textureGather": """
Returns the value

```
vec4(Sample_i0_j1(P, base).comp,
     Sample_i1_j1(P, base).comp,
     Sample_i1_j0(P, base).comp,
     Sample_i0_j0(P, base).comp)
```

If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
""",
        "textureGatherOffset": """
Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
""",
        "textureGatherOffsets": """
Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
""",
    }

    for genPrefix, genPostfix, dim, setup in getAllTextureModifiers():
        genVecType = VectorGenTypeInfo(genPrefix, dim)
        genVecTypeDimPlus1 = VectorGenTypeInfo(genPrefix, min(4, dim + 1))
        genVecTypeDimPlus2 = VectorGenTypeInfo(genPrefix, min(4, dim + 2))
        genVec4Type = VectorGenTypeInfo(genPrefix, 4)
        genIVecType = VectorGenTypeInfo(genPrefix, dim)
        genSamplerParam = f"{genPrefix}sampler{genPostfix} sampler_"

        # 8.9.1 Texture Query Functions
        builder.addFunction(f"{genIVecType} textureSize({genSamplerParam}, int lod);", documentations["textureSize"])
        if not setup.hasRect and not setup.hasBuffer and not setup.hasMS:
            builder.addFunction(f"vec2 textureQueryLod({genSamplerParam}, {genVecType} P);", documentations["textureQueryLod"])
        if not setup.hasRect and not setup.hasBuffer and not setup.hasMS:
            builder.addFunction(f"int textureQueryLevels({genSamplerParam})", documentations["textureQueryLevels"])
        if setup.hasMS:
            builder.addFunction(f"int textureSamples({genSamplerParam});", documentations["textureSamples"])

        # 8.9.2 Texel Lookup Functions
        texLookupReturnType = "float" if setup.hasShadow else genVec4Type
        texLookupPType = genVecType
        match setup.hasArray, setup.hasShadow:
            case True, True:
                texLookupPType = VectorGenTypeInfo(genPrefix, min(4, dim + 2))
            case True, False:
                texLookupPType = VectorGenTypeInfo(genPrefix, min(4, dim + 1))
            case False, True:
                texLookupPType = VectorGenTypeInfo(genPrefix, max(3, min(4, dim + 1)))
            case False, False:
                texLookupPType = genVecType
        texLookupOffsetType = genVecType.getIType()

        texLookupPParam = f", {texLookupPType} P"
        texLookupIntPParam = f", {texLookupPType.getIType()} P"
        texLookupOffsetParam = f", {texLookupOffsetType} offset"
        texLookupLodParam = f", float lod"
        texLookupIntLodParam = f", int lod"
        texLookupDerivativeParam = f", {genVecType} dPdx, {genVecType} dPdy"

        if not setup.hasBuffer and not setup.hasMS:
            compareParam = ", float compare" if setup.hasCube and setup.hasArray and setup.hasShadow else ""
            builder.addFunction(f"{texLookupReturnType} texture({genSamplerParam}{texLookupPParam}{compareParam})", documentations["texture"])
            # builder.addFunction(f"{texLookupReturnType} texture({genSamplerParam}, {texLookupPType} P, float bias)", documentations["texture"])

        if not setup.hasCube and not setup.hasArray and not setup.hasBuffer and not setup.hasMS:
            builder.addFunction(f"{texLookupReturnType} textureProj({genSamplerParam}{texLookupPParam})", documentations["textureProj"])
            # builder.addFunction(f"{texLookupReturnType} textureProj({genSamplerParam}, {texLookupPType} P, float bias)", documentations["textureProj"])

        if not setup.hasRect and not setup.hasBuffer and not setup.hasMS:
            builder.addFunction(f"{texLookupReturnType} textureLod({genSamplerParam}{texLookupPParam}{texLookupLodParam})", documentations["textureLod"])

        if not setup.hasCube and not setup.hasBuffer and not setup.hasMS:
            builder.addFunction(f"{texLookupReturnType} textureOffset({genSamplerParam}{texLookupPParam}{texLookupOffsetParam})", documentations["textureOffset"])
            # builder.addFunction(f"{texLookupReturnType} textureOffset({genSamplerParam}, {texLookupPType} P, {genIVecType} offset, float bias)", documentations["textureOffset"])

        if not setup.hasCube and not setup.hasShadow:
            thirdParamName = "sample" if setup.hasMS else "lod"
            builder.addFunction(f"{texLookupReturnType} texelFetch({genSamplerParam}, {texLookupPType.getIType()} P, int {thirdParamName})", documentations["texelFetch"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasArray=True, hasRect=True)):
            lodParam = ", int lod" if not setup.hasRect else ""
            builder.addFunction(f"{texLookupReturnType} texelFetchOffset({genSamplerParam}{texLookupIntPParam}{lodParam}{texLookupOffsetParam})", documentations["texelFetchOffset"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasShadow=True, hasRect=True)):
            builder.addFunction(f"{texLookupReturnType} textureProjOffset({genSamplerParam}{texLookupPParam}{texLookupOffsetParam})", documentations["textureProjOffset"])
            # builder.addFunction(f"{genVec4Type} textureProjOffset({genSamplerParam}{texLookupPParam}{texLookupOffsetParam}, int bias)", documentations["textureProjOffset"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasArray=True, hasShadow=True)):
            builder.addFunction(f"{texLookupReturnType} textureLodOffset({genSamplerParam}{texLookupPParam}{texLookupLodParam}{texLookupOffsetParam})", documentations["textureLodOffset"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasShadow=True)):
            if setup.hasShadow:
                builder.addFunction(f"{texLookupReturnType} textureProjLod({genSamplerParam}, vec4 P{texLookupLodParam})", documentations["textureProjLod"])
            else:
                builder.addFunction(f"{texLookupReturnType} textureProjLod({genSamplerParam}, {genVecTypeDimPlus1} P{texLookupLodParam})", documentations["textureProjLod"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasShadow=True)):
            if setup.hasShadow:
                builder.addFunction(f"{texLookupReturnType} textureProjLodOffset({genSamplerParam}, vec4 P{texLookupLodParam}{texLookupOffsetParam})", documentations["textureProjLodOffset"])
            else:
                builder.addFunction(f"{texLookupReturnType} textureProjLodOffset({genSamplerParam}, {genVecTypeDimPlus1} P{texLookupLodParam}{texLookupOffsetParam})", documentations["textureProjLodOffset"])

        if not setup.hasBuffer and not setup.hasMS:
            builder.addFunction(f"{texLookupReturnType} textureGrad({genSamplerParam}{texLookupPParam}{texLookupDerivativeParam})", documentations["textureGrad"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasArray=True, hasShadow=True, hasRect=True)):
            builder.addFunction(f"{texLookupReturnType} textureGradOffset({genSamplerParam}{texLookupPParam}{texLookupDerivativeParam}{texLookupOffsetParam})", documentations["textureGradOffset"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasShadow=True, hasRect=True)):
            builder.addFunction(f"{texLookupReturnType} textureProjGrad({genSamplerParam}{texLookupPParam}{texLookupLodParam}{texLookupDerivativeParam})", documentations["textureProjGrad"])

        if setup.test(TextureConfig(has1D=True, has2D=True, has3D=True, hasShadow=True, hasRect=True)):
            builder.addFunction(f"{texLookupReturnType} textureProjGradOffset({genSamplerParam}{texLookupPParam}{texLookupLodParam}{texLookupDerivativeParam}{texLookupOffsetParam})", documentations["textureProjGradOffset"])

        # 8.9.4 Texture Gather Functions
        texGatherPType = genVecTypeDimPlus1 if setup.hasArray else genVecType

        texGatherPParam = f", {texGatherPType} P"
        texGatherRefZParam = ", float refZ" if setup.hasShadow else ""

        if setup.test(TextureConfig(has2D=True, hasCube=True, hasArray=True, hasShadow=True, hasRect=True)):
            if setup.hasShadow:
                builder.addFunction(f"{genVec4Type} textureGather({genSamplerParam}{texGatherPParam}{texGatherRefZParam})", documentations["textureGather"])
            else:
                builder.addFunction(f"{genVec4Type} textureGather({genSamplerParam}{texGatherPParam})", documentations["textureGather"])
                builder.addFunction(f"{genVec4Type} textureGather({genSamplerParam}{texGatherPParam}, int comp)", documentations["textureGather"])

        if setup.test(TextureConfig(has2D=True, hasArray=True, hasShadow=True, hasRect=True)):
            builder.addFunction(f"{genVec4Type} textureGatherOffset({genSamplerParam}{texGatherPParam}{texGatherRefZParam}, ivec2 offset)", documentations["textureGatherOffset"])

        if setup.test(TextureConfig(has2D=True, hasArray=True, hasShadow=True, hasRect=True)):
            builder.addFunction(f"{genVec4Type} textureGatherOffsets({genSamplerParam}{texGatherPParam}{texGatherRefZParam}, ivec2 offset[4])", documentations["textureGatherOffsets"])



    # 8.9.5 Compatibility Profile Texture Functions
    # compat_texture_documentation = "Use the texture coordinate P to do a texture lookup in the texture bound to sampler."
    # compat_guard = ["GL_ARB_shading_language_420pack"]
    # builder.addFunction("vec4 texture1D(sampler1D sampler, float coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1D(sampler1D sampler, float coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DProj(sampler1D sampler, vec2 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DProj(sampler1D sampler, vec2 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DProj(sampler1D sampler, vec4 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DProj(sampler1D sampler, vec4 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DLod(sampler1D sampler, float coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DProjLod(sampler1D sampler, vec2 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture1DProjLod(sampler1D sampler, vec4 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2D(sampler2D sampler, vec2 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2D(sampler2D sampler, vec2 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DProj(sampler2D sampler, vec3 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DProj(sampler2D sampler, vec3 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DProj(sampler2D sampler, vec4 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DProj(sampler2D sampler, vec4 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DLod(sampler2D sampler, vec2 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DProjLod(sampler2D sampler, vec3 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture2DProjLod(sampler2D sampler, vec4 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture3D(sampler3D sampler, vec3 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture3D(sampler3D sampler, vec3 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture3DProj(sampler3D sampler, vec4 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture3DProj(sampler3D sampler, vec4 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture3DLod(sampler3D sampler, vec3 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 texture3DProjLod(sampler3D sampler, vec4 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 textureCube(samplerCube sampler, vec3 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 textureCube(samplerCube sampler, vec3 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 textureCubeLod(samplerCube sampler, vec3 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow1D(sampler1DShadow sampler, vec3 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow1D(sampler1DShadow sampler, vec3 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow2D(sampler2DShadow sampler, vec3 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow2D(sampler2DShadow sampler, vec3 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow1DProj(sampler1DShadow sampler, vec4 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow1DProj(sampler1DShadow sampler, vec4 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow2DProj(sampler2DShadow sampler, vec4 coord)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow2DProj(sampler2DShadow sampler, vec4 coord, float bias)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow1DLod(sampler1DShadow sampler, vec3 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow2DLod(sampler2DShadow sampler, vec3 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow1DProjLod(sampler1DShadow sampler, vec4 coord, float lod)", compat_texture_documentation, compat_guard)
    # builder.addFunction("vec4 shadow2DProjLod(sampler2DShadow sampler, vec4 coord, float lod)", compat_texture_documentation, compat_guard)


# 8.10 Atomic Counter Functions
def addAtomicCounterFunctions(builder: StdlibBuilder):
    documentations = {
        "atomicCounterIncrement": """
Atomically

1. increments the counter for *c*, and
2. returns its value prior to the increment operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterDecrement": """
Atomically

1. decrements the counter for *c*, and
2. returns the value resulting from the decrement operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounter": """
Returns the counter value for *c*.
""",
        "atomicCounterAdd": """
Atomically

1. adds the value of *data* to the counter for *c*, and
2. returns its value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterSubtract": """
Atomically

1. subtracts the value of *data* from the counter for *c*, and
2. returns its value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterMin": """
Atomically

1. sets the counter for *c* to the minimum of the value of the counter and the value of *data*, and
2. returns the value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterMax": """
Atomically

1. sets the counter for *c* to the maximum of the value of the counter and the value of *data*, and
2. returns the value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterAnd": """
Atomically

1. sets the counter for *c* to the bitwise AND of the value of the counter and the value of *data*, and
2. returns the value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterOr": """
Atomically

1. sets the counter for *c* to the bitwise OR of the value of the counter and the value of *data*, and
2. returns the value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterXor": """
Atomically

1. sets the counter for *c* to the bitwise XOR of the value of the counter and the value of *data*, and
2. returns the value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterExchange": """
Atomically

1. sets the counter value for *c* to the value of *data*, and
2. returns its value prior to the operation.

These two steps are done atomically with respect to the atomic counter functions in this table.
""",
        "atomicCounterCompSwap": """
Atomically

1. compares the value of *compare* and the counter value for *c*
2. if the values are equal, sets the counter value for *c* to the value of *data*, and
3. returns its value prior to the operation.

These three steps are done atomically with respect to the atomic counter functions in this table.
"""
    }

    builder.addFunction("uint atomicCounterIncrement(atomic_uint c);", documentations["atomicCounterIncrement"])
    builder.addFunction("uint atomicCounterDecrement(atomic_uint c);", documentations["atomicCounterDecrement"])
    builder.addFunction("uint atomicCounter(atomic_uint c);", documentations["atomicCounter"])
    builder.addFunction("uint atomicCounterAdd(atomic_uint c, uint data);", documentations["atomicCounterAdd"])
    builder.addFunction("uint atomicCounterSubtract(atomic_uint c, uint data);", documentations["atomicCounterSubtract"])
    builder.addFunction("uint atomicCounterMin(atomic_uint c, uint data);", documentations["atomicCounterMin"])
    builder.addFunction("uint atomicCounterMax(atomic_uint c, uint data);", documentations["atomicCounterMax"])
    builder.addFunction("uint atomicCounterAnd(atomic_uint c, uint data);", documentations["atomicCounterAnd"])
    builder.addFunction("uint atomicCounterOr(atomic_uint c, uint data);", documentations["atomicCounterOr"])
    builder.addFunction("uint atomicCounterXor(atomic_uint c, uint data);", documentations["atomicCounterXor"])
    builder.addFunction("uint atomicCounterExchange(atomic_uint c, uint data);", documentations["atomicCounterExchange"])
    builder.addFunction("uint atomicCounterCompSwap(atomic_uint c, uint compare, uint data);", documentations["atomicCounterCompSwap"])

# 8.11 Atomic Memory Functions
def addAtomicMemoryFunctions(builder: StdlibBuilder):
    documentations = {
        "atomicAdd": """
Computes a new value by adding the value of *data* to the contents of *mem*.
""",
        "atomicMin": """
Computes a new value by taking the minimum of the value of *data* and the contents of *mem*.
""",
        "atomicMax": """
Computes a new value by taking the maximum of the value of *data* and the contents of *mem*.
""",
        "atomicAnd": """
Computes a new value by performing a bit-wise AND of the value of *data* and the contents of *mem*.
""",
        "atomicOr": """
Computes a new value by performing a bit-wise OR of the value of *data* and the contents of *mem*.
""",
        "atomicXor": """
Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of *mem*.
""",
        "atomicExchange": """
Computes a new value by simply copying the value of *data*.
""",
        "atomicCompSwap": """
Compares the value of *compare* and the contents of *mem*. If the values are equal, the new value is given by *data*; otherwise, it is taken from the original contents of *mem*.
"""
    }

    builder.addFunction("uint atomicAdd(inout uint mem, uint data);", documentations["atomicAdd"])
    builder.addFunction("int atomicAdd(inout int mem, int data);", documentations["atomicAdd"])

    builder.addFunction("uint atomicMin(inout uint mem, uint data);", documentations["atomicMin"])
    builder.addFunction("int atomicMin(inout int mem, int data);", documentations["atomicMin"])

    builder.addFunction("uint atomicMax(inout uint mem, uint data);", documentations["atomicMax"])
    builder.addFunction("int atomicMax(inout int mem, int data);", documentations["atomicMax"])

    builder.addFunction("uint atomicAnd(inout uint mem, uint data);", documentations["atomicAnd"])
    builder.addFunction("int atomicAnd(inout int mem, int data);", documentations["atomicAnd"])

    builder.addFunction("uint atomicOr(inout uint mem, uint data);", documentations["atomicOr"])
    builder.addFunction("int atomicOr(inout int mem, int data);", documentations["atomicOr"])

    builder.addFunction("uint atomicXor(inout uint mem, uint data);", documentations["atomicXor"])
    builder.addFunction("int atomicXor(inout int mem, int data);", documentations["atomicXor"])

    builder.addFunction("uint atomicExchange(inout uint mem, uint data);", documentations["atomicExchange"])
    builder.addFunction("int atomicExchange(inout int mem, int data);", documentations["atomicExchange"])

    builder.addFunction("uint atomicCompSwap(inout uint mem, uint compare, uint data);", documentations["atomicCompSwap"])
    builder.addFunction("int atomicCompSwap(inout int mem, int compare, int data);", documentations["atomicCompSwap"])

# 8.12 Image Functions
def addImageFunctions(builder: StdlibBuilder):
    documentations = {
        "imageSize": """
Returns the dimensions of the image or images bound to *image*.
For arrayed images, the last component of the return value will hold the size of the array.
Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.

Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
""",
        "imageSamples": """
Returns the number of samples in the image bound to *image*.
""",
        "imageLoad": """
Loads the texel at the coordinate *P* from the image unit *image*.
For multisample loads, the sample number is given by *sample*.
When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
""",
        "imageStore": """
Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
For multisample stores, the sample number is given by *sample*.
When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
""",
        "imageAtomicAdd": """
Computes a new value by adding the value of *data* to the contents of the selected texel.
""",
        "imageAtomicMin": """
Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
""",
        "imageAtomicMax": """
Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
""",
        "imageAtomicAnd": """
Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
""",
        "imageAtomicOr": """    
Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
""",
        "imageAtomicXor": """
Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
""",
        "imageAtomicExchange": """
Computes a new value by simply copying the value of *data* into the selected texel.
""",
        "imageAtomicCompSwap": """
Compares the value of *compare* and the contents of the selected texel.
If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
"""
    }

    for prefix, postfix, dim, config in getAllTextureModifiers():
        if config.hasShadow:
            continue  # Images do not support shadow samplers

        genImageParam = f"{prefix}image{postfix} image_"
        genVec4Type = VectorGenTypeInfo(prefix, 4)
        genIVecType = VectorGenTypeInfo(prefix, dim).getIType()

        genImageFollowingParams = f"{genIVecType} P"
        if config.hasMS:
            genImageFollowingParams += ", int sample"

        builder.addFunction(f"{genIVecType} imageSize(readonly writeonly {genImageParam});", documentations["imageSize"])
        if config.hasMS:
            builder.addFunction(f"int imageSamples(readonly writeonly {genImageParam});", documentations["imageSamples"])

        builder.addFunction(f"{genVec4Type} imageLoad(readonly {genImageParam}, {genImageFollowingParams});", documentations["imageLoad"])
        builder.addFunction(f"void imageStore(writeonly {genImageParam}, {genImageFollowingParams}, {genVec4Type} data);", documentations["imageStore"])
        for intType in ["int", "uint"]:
            builder.addFunction(f"{intType} imageAtomicAdd({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicAdd"])
            builder.addFunction(f"{intType} imageAtomicMin({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicMin"])
            builder.addFunction(f"{intType} imageAtomicMax({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicMax"])
            builder.addFunction(f"{intType} imageAtomicAnd({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicAnd"])
            builder.addFunction(f"{intType} imageAtomicOr({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicOr"])
            builder.addFunction(f"{intType} imageAtomicXor({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicXor"])
            builder.addFunction(f"{intType} imageAtomicExchange({genImageParam}, {genImageFollowingParams}, {intType} data);", documentations["imageAtomicExchange"])
            builder.addFunction(f"{intType} imageAtomicCompSwap({genImageParam}, {genImageFollowingParams}, {intType} compare, {intType} data);", documentations["imageAtomicCompSwap"])

        builder.addFunction(f"float imageAtomicExchange({genImageParam}, {genImageFollowingParams}, float data);", documentations["imageAtomicExchange"])

# 8.13 Geometry Shader Functions
def addGeometryShaderFunctions(builder: StdlibBuilder):
    documentations = {
        "EmitStreamVertex": """
Emits the current values of output variables to the current output primitive on stream *stream*.
The argument to *stream* must be a constant integral expression.
On return from this call, the values of all output variables are undefined.
Can only be used if multiple output streams are supported.
""",
        "EndStreamPrimitive": """
Completes the current output primitive on stream *stream* and starts a new one.
The argument to *stream* must be a constant integral expression.
No vertex is emitted.
Can only be used if multiple output streams are supported.
""",
        "EmitVertex": """
Emits the current values of output variables to the current output primitive.
When multiple output streams are supported, this is equivalent to calling **EmitStreamVertex**(0).
On return from this call, the values of output variables are undefined.
""",
        "EndPrimitive": """
Completes the current output primitive and starts a new one.
When multiple output streams are supported, this is equivalent to calling **EndStreamPrimitive**(0).
No vertex is emitted.
"""
    }

    builder.addFunction("void EmitStreamVertex(int stream);", documentations["EmitStreamVertex"])
    builder.addFunction("void EndStreamPrimitive(int stream);", documentations["EndStreamPrimitive"])
    builder.addFunction("void EmitVertex();", documentations["EmitVertex"])
    builder.addFunction("void EndPrimitive();", documentations["EndPrimitive"])

# 8.14 Fragment Processing Functions
def addFragmentProcessingFunctions(builder: StdlibBuilder):
    # 8.14.1 Derivative Functions
    derivative_documentations = {
        "dFdx": """
Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
""",
        "dFdy": """
Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
""",
        "dFdxFine": """
Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
""",
        "dFdyFine": """
Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
""",
        "dFdxCoarse": """
Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
""",
        "dFdyCoarse": """
Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
""",
        "fwidth": """
Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
""",
        "fwidthFine": """
Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
""",
        "fwidthCoarse": """
Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
"""
    }

    for genType in allVectorGenTypes(GEN_FTYPE_PREFIXES):
        builder.addFunction(f"{genType} dFdx({genType} p);", derivative_documentations["dFdx"], genType.guardMacros)
        builder.addFunction(f"{genType} dFdy({genType} p);", derivative_documentations["dFdy"], genType.guardMacros)
        builder.addFunction(f"{genType} dFdxFine({genType} p);", derivative_documentations["dFdxFine"], genType.guardMacros)
        builder.addFunction(f"{genType} dFdyFine({genType} p);", derivative_documentations["dFdyFine"], genType.guardMacros)
        builder.addFunction(f"{genType} dFdxCoarse({genType} p);", derivative_documentations["dFdxCoarse"], genType.guardMacros)
        builder.addFunction(f"{genType} dFdyCoarse({genType} p);", derivative_documentations["dFdyCoarse"], genType.guardMacros)
        builder.addFunction(f"{genType} fwidth({genType} p);", derivative_documentations["fwidth"], genType.guardMacros)
        builder.addFunction(f"{genType} fwidthFine({genType} p);", derivative_documentations["fwidthFine"], genType.guardMacros)
        builder.addFunction(f"{genType} fwidthCoarse({genType} p);", derivative_documentations["fwidthCoarse"], genType.guardMacros)

    # 8.14.2 Interpolation Functions
    interpolation_documentations = {
        "interpolateAtCentroid": """
Returns the value of the input *interpolant* sampled at a location inside both the pixel and the primitive being processed. The value obtained would be the same value assigned to the input variable if declared with the **centroid** qualifier.
""",
        "interpolateAtSample": """
Returns the value of the input *interpolant* variable at the location of sample number *sample*. If multisample buffers are not available, the input variable will be evaluated at the center of the pixel. If sample *sample* does not exist, the position used to interpolate the input variable is undefined.
""",
        "interpolateAtOffset": """
Returns the value of the input *interpolant* variable sampled at an offset from the center of the pixel specified by *offset*. The two floating-point components of *offset*, give the offset in pixels in the *x* and *y* directions, respectively. An offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets supported by this function is implementation-dependent.
"""
    }

    for genType in allVectorGenTypes([""]):
        builder.addFunction(f"{genType} interpolateAtCentroid({genType} interpolant);", interpolation_documentations["interpolateAtCentroid"])
        builder.addFunction(f"{genType} interpolateAtSample({genType} interpolant, int sample);", interpolation_documentations["interpolateAtSample"])
        builder.addFunction(f"{genType} interpolateAtOffset({genType} interpolant, vec2 offset);", interpolation_documentations["interpolateAtOffset"])

# 8.15 Noise Functions
def addNoiseFunctions(builder: StdlibBuilder):
    # Deprecated
    pass

# 8.16 Shader Invocation Functions
def addShaderInvocationFunctions(builder: StdlibBuilder):
    documentation = {
        "barrier": """
For any given static instance of **barrier**(), all tessellation control shader invocations for a single input patch must enter it before any will be allowed to continue beyond it, or all compute shader invocations for a single workgroup must enter it before any will continue beyond it.
"""
    }
    
    builder.addFunction("void barrier();", documentation["barrier"])

# 8.17 Shader Memory Functions
def addShaderMemoryFunctions(builder: StdlibBuilder):
    documentations = {
        "memoryBarrier": """
Control the ordering of memory transactions issued by a single shader invocation.

When called, this function will wait for the completion of all reads and writes to atomic counter, buffer, image, and shared variables previously performed by the caller, and then return with no other effect.
""",
        "memoryBarrierAtomicCounter": """
Control the ordering of accesses to atomic-counter variables issued by a single shader invocation.

When called, this function will wait for the completion of all reads and writes to atomic counter variables previously performed by the caller, and then return with no other effect.
""",
        "memoryBarrierBuffer": """
Control the ordering of memory transactions to buffer variables issued within a single shader invocation.

When called, this function will wait for the completion of all reads and writes to buffer variables previously performed by the caller, and then return with no other effect.
""",
        "memoryBarrierShared": """
Control the ordering of memory transactions to shared variables issued within a single shader invocation, as viewed by other invocations in the same workgroup.

When called, this function will wait for the completion of all reads and writes to shared variables previously performed by the caller, and then return with no other effect.

Only available in compute shaders.
""",
        "memoryBarrierImage": """
Control the ordering of memory transactions to images issued within a single shader invocation.

When called, this function will wait for the completion of all reads and writes to image variables previously performed by the caller, and then return with no other effect.
""",
        "groupMemoryBarrier": """
Control the ordering of all memory transactions issued within a single shader invocation, as viewed by other invocations in the same workgroup.

When called, this function will wait for the completion of all reads and writes to atomic counter, buffer, image, and shared variables previously performed by the caller, and then return with no other effect.

Only available in compute shaders.
"""
    }

    builder.addFunction("void memoryBarrier();", documentations["memoryBarrier"])
    builder.addFunction("void memoryBarrierAtomicCounter();", documentations["memoryBarrierAtomicCounter"])
    builder.addFunction("void memoryBarrierBuffer();", documentations["memoryBarrierBuffer"])
    builder.addFunction("void memoryBarrierShared();", documentations["memoryBarrierShared"])
    builder.addFunction("void memoryBarrierImage();", documentations["memoryBarrierImage"])
    builder.addFunction("void groupMemoryBarrier();", documentations["groupMemoryBarrier"])

# 8.18 Subpass-Input Functions
def addSubpassInputFunctions(builder: StdlibBuilder):
    documentation = {
        "subpassLoad": """
Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
"""
    }

    for gen in ["", "i", "u"]:
        builder.addFunction(f"{gen}vec4 subpassLoad({gen}subpassInput subpass);", documentation["subpassLoad"])
        builder.addFunction(f"{gen}vec4 subpassLoad({gen}subpassInputMS subpass, int sample);", documentation["subpassLoad"])

# 8.19 Shader Invocation Group Functions
def addShaderInvocationGroupFunctions(builder: StdlibBuilder):
    documentations = {
        "anyInvocation": """
Returns true if and only if value is true for at least one active invocation in the group.
""",
        "allInvocations": """
Returns true if and only if value is true for all active invocations in the group.
""",
        "allInvocationsEqual": """
Returns true if value is the same for all active invocations in the group.
"""
    }
    builder.addFunction("bool anyInvocation(bool value);", documentations["anyInvocation"])
    builder.addFunction("bool allInvocations(bool value);", documentations["allInvocations"])
    builder.addFunction("bool allInvocationsEqual(bool value);", documentations["allInvocationsEqual"])