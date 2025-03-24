#include "Basic/StringView.h"

#include "Server/StandardDocumentation.h"

#include <unordered_map>

namespace glsld
{
    auto QueryLocationQualifierDocumentation(StringView name) -> StringView
    {
        static const std::unordered_map<StringView, StringView> documentations = {
            // Tesselation
            //

            {
                "triangles",
                "The `primitive-mode` is used to specify a tessellation primitive mode to be used by the tessellation "
                "primitive generator.",
            },
            {
                "quads",
                "The `primitive-mode` is used to specify a tessellation primitive mode to be used by the tessellation "
                "primitive generator.",
            },
            {
                "isolines",
                "The `primitive-mode` is used to specify a tessellation primitive mode to be used by the tessellation "
                "primitive generator.",
            },
            {
                "equal_spacing",
                "`equal_spacing` specifies that edges should be divided into a collection of equal-sized segments",
            },
            {
                "fractional_even_spacing",
                "`fractional_even_spacing` specifies that edges should be divided into an even number of equal-length "
                "segments plus two additional shorter \"fractional\" segments",
            },
            {
                "fractional_odd_spacing",
                "`fractional_odd_spacing` specifies that edges should be divided into an odd number of equal-length "
                "segments plus two additional shorter \"fractional\" segments",
            },
            {
                "cw",
                "`cw` indicate clockwise triangles,  If the tessellation primitive generator does not produce "
                "triangles, the order is ignored",
            },
            {
                "ccw",
                "`ccw` indicate counter-clockwise triangles,  If the tessellation primitive generator does not produce "
                "triangles, the order is ignored",
            },
            {
                "point_mode",
                "`point_mode` indicates that the tessellation primitive generator should produce one point for each "
                "distinct vertex in the subdivided primitive, rather than generating lines or triangles",
            },
            {
                "points",
                "`points` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "lines",
                "`lines` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "lines_adjacency",
                "`lines_adjacency` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "triangles",
                "`triangles` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "triangles_adjacency",
                "`triangles_adjacency` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "invocations",
                "`invocations = layout-qualifier-value` is used to specify the number of times the geometry shader "
                "executable is invoked for each input primitive received. Invocation count declarations are optional. "
                "If no invocation count is declared in any geometry shader in a program, the geometry shader will be "
                "run once for each input primitive.",
            },

            // Fragment
            //

            // {
            //     "origin_upper_left",
            //     "`origin_upper_left` ",
            // },
            // {
            //     "pixel_center_integer",
            //     "`pixel_center_integer` ",
            // },
            // {
            //     "early_fragment_tests",
            //     "`early_fragment_tests` ",
            // },

            // Compute
            //
            {
                "local_size_x",
                "`local_size_x` is used to declare a fixed workgroup size by the compute shader in the first "
                "dimension. If a shader does not specify a size for one of the dimensions, that dimension will have a "
                "size of 1.",
            },
            {
                "local_size_y",
                "`local_size_y` is used to declare a fixed workgroup size by the compute shader in the second "
                "dimension. If a shader does not specify a size for one of the dimensions, that dimension will have a "
                "size of 1.",
            },
            {
                "local_size_y",
                "`local_size_y` is used to declare a fixed workgroup size by the compute shader in the third "
                "dimension. If a shader does not specify a size for one of the dimensions, that dimension will have a "
                "size of 1.",
            },

            // Output
            //
            {
                "location",
                "`location` ",
            },
            {
                "component",
                "`component` ",
            },
            {
                "index",
                "`index` ",
            },

            // Transform Feedback
            //
            {
                "xfb_buffer",
                "The `xfb_buffer` qualifier specifies which transform feedback buffer will capture outputs selected "
                "with `xfb_offset`. The `xfb_buffer` qualifier can be applied to the qualifier out, to output "
                "variables, to output blocks, and to output block members.",
            },
            {
                "xfb_offset",
                "`xfb_offset` The xfb_offset qualifier assigns a byte offset within a transform feedback buffer. Only "
                "variables, block members, or blocks can be qualified with xfb_offset.",
            },
            {
                "xfb_stride",
                "`xfb_stride` The xfb_stride qualifier specifies how many bytes are consumed by each captured vertex. "
                "It applies to the transform feedback buffer for that declaration, whether it is inherited or "
                "explicitly declared.",
            },

            // Tessellation Control Outputs
            //
            {
                "vertices",
                "`vertices` The identifier vertices specifies the number of vertices in the output patch produced by "
                "the tessellation control shader, which also specifies the number of times the tessellation control "
                "shader is invoked. It is a compile- or link-time error for the output vertex count to be less than or "
                "equal to zero, or greater than the implementation-dependent maximum patch size.",
            },

            // Geometry Outputs
            //
            // {
            //     "points",
            //     "`points` ",
            // },
            // {
            //     "line_strip",
            //     "`line_strip` ",
            // },
            // {
            //     "triangle_strip",
            //     "`triangle_strip` ",
            // },
            // {
            //     "max_vertices",
            //     "`max_vertices` ",
            // },
            // {
            //     "stream",
            //     "`stream` ",
            // },

            // Fragment Outputs
            //
            // {
            //     "depth_any",
            //     "`depth_any` ",
            // },
            // {
            //     "depth_greater",
            //     "`depth_greater` ",
            // },
            // {
            //     "depth_less",
            //     "`depth_less` ",
            // },
            // {
            //     "depth_unchanged",
            //     "`depth_unchanged` ",
            // },
        };

        if (auto it = documentations.find(name); it != documentations.end()) {
            return it->second;
        }
        else {
            return "";
        }
    }

    auto QueryFunctionDocumentation(StringView name) -> StringView
    {
        static const std::unordered_map<StringView, StringView> documentations = {
            // clang-format off
{
    "atomicCounter",
    "Returns the counter value for c.",
},
{
    "atomicCounterAdd",
    "Atomically\n\n1. adds the value of data to the counter for c, and\n2. returns its value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterAnd",
    "Atomically\n\n1. sets the counter for c to the bitwise AND of the value of the counter and the value of data, and\n2. returns the value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterCompSwap",
    "\nAtomically\n\n1. compares the value of compare and the counter value for c\n2. if the values are equal, sets the counter value for c to the value of data, and\n3. returns its value prior to the operation.\n\nThese three steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterDecrement",
    "Atomically\n\n1. decrements the counter for c, and\n2. returns the value resulting from the decrement operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterExchange",
    "Atomically\n\n1. sets the counter value for c to the value of data, and\n2. returns its value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterIncrement",
    "Atomically\n\n1. increments the counter for c, and\n2. returns its value prior to the increment operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterMax",
    "Atomically\n\n1. sets the counter for c to the maximum of the value of the counter and the value of data, and\n2. returns the value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterMin",
    "Atomically\n\n1. sets the counter for c to the minimum of the value of the counter and the value of data, and\n2. returns the value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterOr",
    "Atomically\n\n1. sets the counter for c to the bitwise OR of the value of the counter and the value of data, and\n2. returns the value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterSubtract",
    "Atomically\n\n1. subtracts the value of data from the counter for c, and\n2. returns its value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicCounterXor",
    "Atomically\n\n1. sets the counter for c to the bitwise XOR of the value of the counter and the value of data, and\n2. returns the value prior to the operation.\n\nThese two steps are done atomically with respect to the atomic counter functions in this table.",
},
{
    "atomicAdd",
    "Computes a new value by adding the value of data to the contents mem.",
},
{
    "atomicAnd",
    "Computes a new value by performing a bit-wise AND of the value of data and the contents of mem.",
},
{
    "atomicCompSwap",
    "Compares the value of compare and the contents of mem. If the values are equal, the new value is given by data; otherwise, it is taken from the original contents of mem.",
},
{
    "atomicExchange",
    "Computes a new value by simply copying the value of data.",
},
{
    "atomicMax",
    "Computes a new value by taking the maximum of the value of data and the contents of mem.",
},
{
    "atomicMin",
    "Computes a new value by taking the minimum of the value of data and the contents of mem.",
},
{
    "atomicOr",
    "Computes a new value by performing a bit-wise OR of the value of data and the contents of mem.",
},
{
    "atomicXor",
    "Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of data and the contents of mem.",
},
{
    "abs",
    "Returns x if x ≥ 0; otherwise it returns -x.",
},
{
    "ceil",
    "Returns a value equal to the nearest integer that is greater than or equal to x.",
},
{
    "clamp",
    "Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.",
},
{
    "floatBitsToInt",
    "Returns a signed or unsigned integer value representing the encoding of a floating-point value. The float value’s bit-level representation is preserved.",
},
{
    "floatBitsToUint",
    "Returns a signed or unsigned integer value representing the encoding of a floating-point value. The float value’s bit-level representation is preserved.",
},
{
    "floor",
    "Returns a value equal to the nearest integer that is less than or equal to x.",
},
{
    "fma",
    "Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:\n\n- fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.\n- The precision of fma() can differ from the precision of the expression a * b + c.\n- fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.\n\nOtherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.",
},
{
    "fract",
    "Returns x - floor(x).",
},
{
    "frexp",
    "Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that\n\nx = significant · 2^exponent\n\nThe significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.\n\nIf an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.\n\nIf the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.",
},
{
    "intBitsToFloat",
    "Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. Otherwise, the bit-level representation is preserved.",
},
{
    "isinf",
    "Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.",
},
{
    "isnan",
    "Returns true if x holds a NaN. Returns false otherwise. Always returns false if NaNs are not implemented.",
},
{
    "ldexp",
    "Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:\n\nsignificand · 2^exponent\n\nIf this product is too large to be represented in the floating-point type, the result is undefined.\n\nIf exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-denormalized values.\nIf the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.",
},
{
    "max",
    "Returns y if x < y; otherwise it returns x.",
},
{
    "min",
    "Returns y if y < x; otherwise it returns x.",
},
{
    "mix",
    "Returns the linear blend of x and y, i.e., x · (1 - a) + y · a.",
},
{
    "mix_2",
    "Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example,\n\n```\ngenFType mix(genFType x, genFType y, genFType(a))\n```\n\nwhere a is a Boolean vector.",
},
{
    "mod",
    "Modulus. Returns x - y · floor(x / y).",
},
{
    "modf",
    "Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.",
},
{
    "round",
    "Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.",
},
{
    "roundEven",
    "Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)",
},
{
    "sign",
    "Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.",
},
{
    "smoothstep",
    "Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:\n\n```\ngenFType t;\nt = clamp ((x - edge0) / (edge1 - edge0), 0, 1);\nreturn t * t * (3 - 2 * t);\n```\n\n(And similarly for doubles.) Results are undefined if edge0 ≥ edge1.",
},
{
    "step",
    "Returns 0.0 if x < edge; otherwise it returns 1.0.",
},
{
    "trunc",
    "Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.",
},
{
    "uintBitsToFloat",
    "Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. Otherwise, the bit-level representation is preserved.",
},
{
    "exp",
    "Returns the natural exponentiation of x, i.e., e^x.",
},
{
    "exp2",
    "Returns 2 raised to the x power, i.e., 2^x.",
},
{
    "inversesqrt",
    "Returns 1 / sqrt(x). Results are undefined if x ≤ 0.",
},
{
    "log",
    "Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x ≤ 0.",
},
{
    "log2",
    "Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x ≤ 0.",
},
{
    "pow",
    "Returns x raised to the y power, i.e., xy. Results are undefined if x < 0. Results are undefined if x = 0 and y ≤ 0.",
},
{
    "sqrt",
    "Returns sqrt(x). Results are undefined if x < 0.",
},
{
    "packDouble2x32",
    "Returns a double-precision value obtained by packing the components of v into a 64-bit value. If an IEEE 754 Inf or NaN is created, it will not signal, and the resulting floating-point value is unspecified. Otherwise, the bit-level representation of v is preserved. The first vector component specifies the 32 least significant bits; the second component specifies the 32 most significant bits.",
},
{
    "packHalf2x16",
    "Returns an unsigned integer obtained by converting the components of a two-component floating-point vector to the 16-bit floating-point representation of the API, and then packing these two 16-bit integers into a 32-bit unsigned integer.\n\nThe first vector component specifies the 16 least-significant bits of the result; the second component specifies the 16 most-significant bits.",
},
{
    "packSnorm2x16",
    "First, converts each component of the normalized floating-point value v into 16-bit (2x16) or 8-bit (4x8) integer values. Then, the results are packed into the returned 32-bit unsigned integer.\n\nThe conversion for component c of v to fixed point is done as follows:\n\n- packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)\n- packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)\n- packUnorm4x8: round(clamp(c, 0, +1) * 255.0)\n- packSnorm4x8: round(clamp(c, -1, +1) * 127.0)\n\nThe first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.",
},
{
    "packSnorm4x8",
    "First, converts each component of the normalized floating-point value v into 16-bit (2x16) or 8-bit (4x8) integer values. Then, the results are packed into the returned 32-bit unsigned integer.\n\nThe conversion for component c of v to fixed point is done as follows:\n\n- packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)\n- packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)\n- packUnorm4x8: round(clamp(c, 0, +1) * 255.0)\n- packSnorm4x8: round(clamp(c, -1, +1) * 127.0)\n\nThe first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.",
},
{
    "packUnorm2x16",
    "First, converts each component of the normalized floating-point value v into 16-bit (2x16) or 8-bit (4x8) integer values. Then, the results are packed into the returned 32-bit unsigned integer.\n\nThe conversion for component c of v to fixed point is done as follows:\n\n- packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)\n- packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)\n- packUnorm4x8: round(clamp(c, 0, +1) * 255.0)\n- packSnorm4x8: round(clamp(c, -1, +1) * 127.0)\n\nThe first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.",
},
{
    "packUnorm4x8",
    "First, converts each component of the normalized floating-point value v into 16-bit (2x16) or 8-bit (4x8) integer values. Then, the results are packed into the returned 32-bit unsigned integer.\n\nThe conversion for component c of v to fixed point is done as follows:\n\n- packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)\n- packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)\n- packUnorm4x8: round(clamp(c, 0, +1) * 255.0)\n- packSnorm4x8: round(clamp(c, -1, +1) * 127.0)\n\nThe first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.",
},
{
    "unpackDouble2x32",
    "Returns a two-component unsigned integer vector representation of v. The bit-level representation of v is preserved. The first component of the vector contains the 32 least significant bits of the double; the second component consists of the 32 most significant bits.",
},
{
    "unpackHalf2x16",
    "Returns a two-component floating-point vector with components obtained by unpacking a 32-bit unsigned integer into a pair of 16-bit values, interpreting those values as 16-bit floating-point numbers according to the API, and converting them to 32-bit floating-point values.\n\nThe first component of the vector is obtained from the 16 least-significant bits of v; the second component is obtained from the 16 most-significant bits of v.",
},
{
    "unpackSnorm2x16",
    "First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.\n\nThe conversion for unpacked fixed-point value f to floating-point is done as follows:\n\n- unpackUnorm2x16: f / 65535.0\n- unpackSnorm2x16: clamp(f / 32767.0, -1, +1)\n- unpackUnorm4x8: f / 255.0\n- unpackSnorm4x8: clamp(f / 127.0, -1, +1)\n\nThe first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.",
},
{
    "unpackSnorm4x8",
    "First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.\n\nThe conversion for unpacked fixed-point value f to floating-point is done as follows:\n\n- unpackUnorm2x16: f / 65535.0\n- unpackSnorm2x16: clamp(f / 32767.0, -1, +1)\n- unpackUnorm4x8: f / 255.0\n- unpackSnorm4x8: clamp(f / 127.0, -1, +1)\n\nThe first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.",
},
{
    "unpackUnorm2x16",
    "First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.\n\nThe conversion for unpacked fixed-point value f to floating-point is done as follows:\n\n- unpackUnorm2x16: f / 65535.0\n- unpackSnorm2x16: clamp(f / 32767.0, -1, +1)\n- unpackUnorm4x8: f / 255.0\n- unpackSnorm4x8: clamp(f / 127.0, -1, +1)\n\nThe first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.",
},
{
    "unpackUnorm4x8",
    "First, unpacks a single 32-bit unsigned integer p into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.\n\nThe conversion for unpacked fixed-point value f to floating-point is done as follows:\n\n- unpackUnorm2x16: f / 65535.0\n- unpackSnorm2x16: clamp(f / 32767.0, -1, +1)\n- unpackUnorm4x8: f / 255.0\n- unpackSnorm4x8: clamp(f / 127.0, -1, +1)\n\nThe first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.",
},
{
    "dFdx",
    "Returns either dFdxFine(p) or dFdxCoarse(p), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.",
},
{
    "dFdxCoarse",
    "Returns the partial derivative of p with respect to the window x coordinate. Will use local differencing based on the value of p for the current fragment’s neighbors, and will possibly, but not necessarily, include the value of p for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for dFdxFine(p).",
},
{
    "dFdxFine",
    "Returns the partial derivative of p with respect to the window x coordinate. Will use local differencing based on the value of p for the current fragment and its immediate neighbor(s).",
},
{
    "dFdy",
    "Returns either dFdyFine(p) or dFdyCoarse(p), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.",
},
{
    "dFdyCoarse",
    "Returns the partial derivative of p with respect to the window y coordinate. Will use local differencing based on the value of p for the current fragment’s neighbors, and will possibly, but not necessarily, include the value of p for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for dFdyFine(p).",
},
{
    "dFdyFine",
    "Returns the partial derivative of p with respect to the window y coordinate. Will use local differencing based on the value of p for the current fragment and its immediate neighbor(s).",
},
{
    "fwidth",
    "Returns abs(dFdx(p)) + abs(dFdy(p)).",
},
{
    "fwidthCoarse",
    "Returns abs(dFdxCoarse(p)) + abs(dFdyCoarse(p)).",
},
{
    "fwidthFine",
    "Returns abs(dFdxFine(p)) + abs(dFdyFine(p)).",
},
{
    "interpolateAtCentroid",
    "Returns the value of the input interpolant sampled at a location inside both the pixel and the primitive being processed. The value obtained would be the same value assigned to the input variable if declared with the centroid qualifier.",
},
{
    "interpolateAtOffset",
    "Returns the value of the input interpolant variable sampled at an offset from the center of the pixel specified by offset. The two floating-point components of offset, give the offset in pixels in the x and y directions, respectively.\n\nAn offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets supported by this function is implementation-dependent.",
},
{
    "interpolateAtSample",
    "Returns the value of the input interpolant variable at the location of sample number sample. If multisample buffers are not available, the input variable will be evaluated at the center of the pixel. If sample sample does not exist, the position used to interpolate the input variable is undefined.",
},
{
    "cross",
    "Returns the cross product of x and y, i.e., (x_1 · y_2 - y_1 · x_2, x_2 · y_0 - y_2 · x_0, x_0 · y_1 - y_0 · x_1).",
},
{
    "distance",
    "Returns the distance between p0 and p1, i.e., length(p0 - p1)",
},
{
    "dot",
    "Returns the dot product of x and y, i.e., x_0 · y_0 + x_1 · y_1 + …​",
},
{
    "faceforward",
    "If dot(Nref, I) < 0 return N, otherwise return -N.",
},
{
    "length",
    "Returns the length of vector x, i.e., sqrt( x_0^2 + x_1^2 + …​ ).",
},
{
    "normalize",
    "Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).",
},
{
    "reflect",
    "For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.",
},
{
    "refract",
    "For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector. The result is computed by the refraction equation.\n\n```\nThe input parameters for the incident vector _I_ and the surface normal _N_ must already be normalized to get the desired results.\n```",
},
{
    "EmitStreamVertex",
    "Emits the current values of output variables to the current output primitive on stream stream. The argument to stream must be a constant integral expression. On return from this call, the values of all output variables are undefined.\nCan only be used if multiple output streams are supported.",
},
{
    "EmitVertex",
    "Emits the current values of output variables to the current output primitive. When multiple output streams are supported, this is equivalent to calling EmitStreamVertex(0).\nOn return from this call, the values of output variables are undefined.",
},
{
    "EndPrimitive",
    "Completes the current output primitive and starts a new one. When multiple output streams are supported, this is equivalent to calling EndStreamPrimitive(0).\nNo vertex is emitted.",
},
{
    "EndStreamPrimitive",
    "Completes the current output primitive on stream stream and starts a new one. The argument to stream must be a constant integral expression. No vertex is emitted.\nCan only be used if multiple output streams are supported.",
},
{
    "imageAtomicAdd",
    "Computes a new value by adding the value of data to the contents of the selected texel.",
},
{
    "imageAtomicAnd",
    "Computes a new value by performing a bit-wise AND of the value of data and the contents of the selected texel.",
},
{
    "imageAtomicCompSwap",
    "Compares the value of compare and the contents of the selected texel. If the values are equal, the new value is given by data; otherwise, it is taken from the original value loaded from the texel.",
},
{
    "imageAtomicExchange",
    "Computes a new value by simply copying the value of data.",
},
{
    "imageAtomicMax",
    "Computes a new value by taking the maximum of the value data and the contents of the selected texel.",
},
{
    "imageAtomicMin",
    "Computes a new value by taking the minimum of the value of data and the contents of the selected texel.",
},
{
    "imageAtomicOr",
    "Computes a new value by performing a bit-wise OR of the value of data and the contents of the selected texel.",
},
{
    "imageAtomicXor",
    "Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of data and the contents of the selected texel.",
},
{
    "imageLoad",
    "Loads the texel at the coordinate P from the image unit image (in IMAGE_PARAMS). For multisample loads, the sample number is given by sample. When image, P, and sample identify a valid texel, the bits used to represent the selected texel in memory are converted to a vec4, ivec4, or uvec4 in the manner described in section 8.26 “Texture Image Loads and Stores” of the OpenGL Specification and returned.",
},
{
    "imageSamples",
    "Returns the number of samples of the image or images bound to image.",
},
{
    "imageSize",
    "Returns the dimensions of the image or images bound to image. For arrayed images, the last component of the return value will hold the size of the array. Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.\n\nNote: The qualification readonly writeonly accepts a variable qualified with readonly, writeonly, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.",
},
{
    "imageStore",
    "Stores data into the texel at the coordinate P from the image specified by image. For multisample stores, the sample number is given by sample. When image, P, and sample identify a valid texel, the bits used to represent data are converted to the format of the image unit in the manner described in section 8.26 “Texture Image Loads and Stores” of the OpenGL Specification and stored to the specified texel.",
},
{
    "bitCount",
    "Returns the number of one bits in the binary representation of value.",
},
{
    "bitfieldExtract",
    "Extracts bits [offset, offset + bits - 1] from value, returning them in the least significant bits of the result.\n\nFor unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit offset + bits - 1.\n\nIf bits is zero, the result will be zero. The result will be undefined if offset or bits is negative, or if the sum of offset and bits is greater than the number of bits used to store the operand. Note that for vector versions of bitfieldExtract(), a single pair of offset and bits values is shared for all components.",
},
{
    "bitfieldInsert",
    "Inserts the bits least significant bits of insert into base.\n\nThe result will have bits [offset, offset + bits - 1] taken from bits [0, bits - 1] of insert, and all other bits taken directly from the corresponding bits of base. If bits is zero, the result will simply be base. The result will be undefined if offset or bits is negative, or if the sum of offset and bits is greater than the number of bits used to store the operand.\nNote that for vector versions of bitfieldInsert(), a single pair of offset and bits values is shared for all components.",
},
{
    "bitfieldReverse",
    "Reverses the bits of value. The bit numbered n of the result will be taken from bit (bits - 1) - n of value, where bits is the total number of bits used to represent value.",
},
{
    "findLSB",
    "Returns the bit number of the least significant one bit in the binary representation of value. If value is zero, -1 will be returned.",
},
{
    "findMSB",
    "Returns the bit number of the most significant bit in the binary representation of value.\n\nFor positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a value of zero or negative one, -1 will be returned.",
},
{
    "imulExtended",
    "Multiplies 32-bit unsigned or signed integers x and y, producing a 64-bit result. The 32 least-significant bits are returned in lsb. The 32 most-significant bits are returned in msb.",
},
{
    "uaddCarry",
    "Adds 32-bit unsigned integers x and y, returning the sum modulo 2^32. The value carry is set to zero if the sum was less than 2^32, or one otherwise.",
},
{
    "umulExtended",
    "Multiplies 32-bit unsigned or signed integers x and y, producing a 64-bit result. The 32 least-significant bits are returned in lsb. The 32 most-significant bits are returned in msb.",
},
{
    "usubBorrow",
    "Subtracts the 32-bit unsigned integer y from x, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value borrow is set to zero if x ≥ y, or one otherwise.",
},
{
    "determinant",
    "Returns the determinant of m.",
},
{
    "inverse",
    "Returns a matrix that is the inverse of m. The input matrix m is not modified. The values in the returned matrix are undefined if m is singular or poorly-conditioned (nearly singular).",
},
{
    "matrixCompMult",
    "Multiply matrix x by matrix y component-wise, i.e., result[i][j] is the scalar product of x[i][j] and y[i][j].\n\nNote: to get linear algebraic matrix multiplication, use the multiply operator (*).",
},
{
    "outerProduct",
    "Treats the first parameter c as a column vector (matrix with one column) and the second parameter r as a row vector (matrix with one row) and does a linear algebraic matrix multiply c * r, yielding a matrix whose number of rows is the number of components in c and whose number of columns is the number of components in r.",
},
{
    "transpose",
    "Returns a matrix that is the transpose of m. The input matrix m is not modified.",
},
{
    "barrier",
    "For any given static instance of barrier(), all tessellation control shader invocations for a single input patch must enter it before any will be allowed to continue beyond it, or all compute shader invocations for a single workgroup must enter it before any will continue beyond it.",
},
{
    "allInvocations",
    "Returns true if and only if value is true for all active invocations in the group.",
},
{
    "allInvocationsEqual",
    "Returns true if value is the same for all active invocations in the group.",
},
{
    "anyInvocation",
    "Returns true if and only if value is true for at least one active invocation in the group.",
},
{
    "groupMemoryBarrier",
    "Control the ordering of all memory transactions issued within a single shader invocation, as viewed by other invocations in the same workgroup.\n\nOnly available in compute shaders.",
},
{
    "memoryBarrier",
    "Control the ordering of memory transactions issued by a single shader invocation.",
},
{
    "memoryBarrierAtomicCounter",
    "Control the ordering of accesses to atomic-counter variables issued by a single shader invocation.",
},
{
    "memoryBarrierBuffer",
    "Control the ordering of memory transactions to buffer variables issued within a single shader invocation.",
},
{
    "memoryBarrierImage",
    "Control the ordering of memory transactions to images issued within a single shader invocation.",
},
{
    "memoryBarrierShared",
    "Control the ordering of memory transactions to shared variables issued within a single shader invocation, as viewed by other invocations in the same workgroup.\n\nOnly available in compute shaders.",
},
{
    "subpassLoad",
    "Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.",
},
{
    "texelFetch",
    "Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.",
},
{
    "texelFetchOffset",
    "Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.",
},
{
    "texture",
    "Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.\n\nFor shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)\n\nFor non-shadow forms: the array layer comes from the last component of P.",
},
{
    "textureGather",
    "Returns the value\n\n```\nvec4(Sample_i0_j1(P, base).comp,\n     Sample_i1_j1(P, base).comp,\n     Sample_i1_j0(P, base).comp,\n     Sample_i0_j0(P, base).comp)\n```\n\nIf specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.",
},
{
    "textureGatherOffset",
    "Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.",
},
{
    "textureGatherOffsets",
    "Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.",
},
{
    "textureGrad",
    "Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.",
},
{
    "textureGradOffset",
    "Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.",
},
{
    "textureLod",
    "Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λbase] and sets the partial derivatives as follows:\n(See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)\n\n∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0\n∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0",
},
{
    "textureLodOffset",
    "Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.",
},
{
    "textureOffset",
    "Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.\n\nNote that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).\nNote that texel offsets are also not supported for cube maps.",
},
{
    "textureProj",
    "Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.",
},
{
    "textureProjGrad",
    "Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.",
},
{
    "textureProjGradOffset",
    "Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.",
},
{
    "textureProjLod",
    "Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.",
},
{
    "textureProjLodOffset",
    "Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.",
},
{
    "textureProjOffset",
    "Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.",
},
{
    "textureQueryLevels",
    "Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.\n\nThe value zero will be returned if no texture or an incomplete texture is associated with sampler.\n\nAvailable in all shader stages.",
},
{
    "textureQueryLod",
    "Returns the mipmap array(s) that would be accessed in the x component of the return value.\n\nReturns the computed level-of-detail relative to the base level in the y component of the return value.\n\nIf called on an incomplete texture, the results are undefined.",
},
{
    "textureSamples",
    "Returns the number of samples of the texture or textures bound to sampler.",
},
{
    "textureSize",
    "Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 8.11 “Texture Queries” of the OpenGL Specification.\nThe components in the return value are filled in, in order, with the width, height, and depth of the texture.\n\nFor the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.",
},
{
    "acos",
    "Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0,π]. Results are undefined if |x| > 1.",
},
{
    "acosh",
    "Arc hyperbolic cosine; returns the non-negative inverse of cosh. Results are undefined if x < 1.",
},
{
    "asin",
    "Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.",
},
{
    "asinh",
    "Arc hyperbolic sine; returns the inverse of sinh.",
},
{
    "atan",
    "Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π. Results are undefined if x and y are both 0.",
},
{
    "atanh",
    "Arc hyperbolic tangent; returns the inverse of tanh. Results are undefined if x ≥ 1.",
},
{
    "atan_2",
    "Arc tangent. Returns an angle whose tangent is y_over_x. The range of values returned by this function is [-π / 2, π / 2].",
},
{
    "cos",
    "The standard trigonometric cosine function.",
},
{
    "cosh",
    "Returns the hyperbolic cosine function (e^x + e^-x) / 2.",
},
{
    "degrees",
    "Converts radians to degrees, i.e., (180 / π) · radians.",
},
{
    "radians",
    "Converts degrees to radians, i.e., (π / 180) · degrees.",
},
{
    "sin",
    "The standard trigonometric sine function.",
},
{
    "sinh",
    "Returns the hyperbolic sine function (e^x - e^-x) / 2.",
},
{
    "tan",
    "The standard trigonometric tangent.",
},
{
    "tanh",
    "Returns the hyperbolic tangent function sinh(x) / cosh(x).",
},
{
    "all",
    "Returns true only if all components of x are true.",
},
{
    "any",
    "Returns true if any component of x is true.",
},
{
    "equal",
    "Returns the component-wise compare of x == y.",
},
{
    "greaterThan",
    "Returns the component-wise compare of x > y.",
},
{
    "greaterThanEqual",
    "Returns the component-wise compare of x ≥ y.",
},
{
    "lessThan",
    "Returns the component-wise compare of x < y.",
},
{
    "lessThanEqual",
    "Returns the component-wise compare of x ≤ y.",
},
{
    "not",
    "Returns the component-wise logical complement of x.",
},
{
    "notEqual",
    "Returns the component-wise compare of x ≠ y.",
},
            // clang-format on
        };

        if (auto it = documentations.find(name); it != documentations.end()) {
            return it->second;
        }
        else {
            return "";
        }
    }
} // namespace glsld