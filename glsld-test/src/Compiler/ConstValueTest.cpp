#include "Catch2Wrapper.h"

#include "Language/ConstValue.h"

using namespace glsld;

namespace
{
    template <typename T>
    auto TestConstValue(const ConstValue& value, int rowSize, int columnSize, std::initializer_list<T> expected) -> bool
    {
        if (value.GetScalarKind() != GetScalarKindFromCppType<T>()) {
            // The scalar type doesn't match.
            return false;
        }
        if (value.GetRowSize() != rowSize || value.GetColumnSize() != columnSize) {
            // The shape doesn't match.
            return false;
        }

        return std::ranges::equal(value.GetBufferAs<T>(), expected, [](T actual, T expected) {
            if constexpr (std::floating_point<T>) {
                return actual == Catch::Approx(expected);
            }
            else {
                return actual == expected;
            }
        });
    }

    template <typename T>
    auto TestConstScalar(const ConstValue& value, T expected) -> bool
    {
        return TestConstValue(value, 1, 1, {expected});
    }

    template <typename T>
    auto TestConstVector(const ConstValue& value, std::initializer_list<T> expected) -> bool
    {
        return TestConstValue(value, 1, expected.size(), expected);
    }

    template <typename T>
    auto TestConstMatrix(const ConstValue& value, int rowSize, int columnSize, std::initializer_list<T> expected)
        -> bool
    {
        GLSLD_ASSERT(expected.size() == rowSize * columnSize);
        return TestConstValue(value, rowSize, columnSize, expected);
    }

} // namespace

TEST_CASE("Compiler::ConstValueTest")
{
    SECTION("CreateScalar")
    {
        // Test construction from bool
        ConstValue boolValue = ConstValue::CreateScalar(true);
        REQUIRE(TestConstScalar(boolValue, true));

        // Test construction from int32_t
        ConstValue intValue = ConstValue::CreateScalar(static_cast<int32_t>(42));
        REQUIRE(TestConstScalar(intValue, static_cast<int32_t>(42)));

        // Test construction from uint32_t
        ConstValue uintValue = ConstValue::CreateScalar(static_cast<uint32_t>(42));
        REQUIRE(TestConstScalar(uintValue, static_cast<uint32_t>(42)));

        // Test construction from float
        ConstValue floatValue = ConstValue::CreateScalar(3.14f);
        REQUIRE(TestConstScalar(floatValue, 3.14f));

        // Test construction from double
        ConstValue doubleValue = ConstValue::CreateScalar(2.718);
        REQUIRE(TestConstScalar(doubleValue, 2.718));

        // Test construction from int8_t
        ConstValue int8Value = ConstValue::CreateScalar(int8_t(42));
        REQUIRE(TestConstScalar(int8Value, int8_t(42)));

        // Test construction from uint8_t
        ConstValue uint8Value = ConstValue::CreateScalar(uint8_t(42));
        REQUIRE(TestConstScalar(uint8Value, uint8_t(42)));

        // Test construction from int16_t
        ConstValue int16Value = ConstValue::CreateScalar(int16_t(42));
        REQUIRE(TestConstScalar(int16Value, int16_t(42)));

        // Test construction from uint16_t
        ConstValue uint16Value = ConstValue::CreateScalar(uint16_t(42));
        REQUIRE(TestConstScalar(uint16Value, uint16_t(42)));

        // Test construction from int64_t
        ConstValue int64Value = ConstValue::CreateScalar(int64_t(42));
        REQUIRE(TestConstScalar(int64Value, int64_t(42)));

        // Test construction from uint64_t
        ConstValue uint64Value = ConstValue::CreateScalar(uint64_t(42));
        REQUIRE(TestConstScalar(uint64Value, uint64_t(42)));
    }

    SECTION("CreateVector")
    {
        // Test construction of a 2-element int vector
        ConstValue ivec2 = ConstValue::CreateVector<int32_t>({1, 2});
        REQUIRE(TestConstVector(ivec2, {1, 2}));

        // Test construction of a 3-element int vector
        ConstValue ivec3 = ConstValue::CreateVector<int32_t>({1, 2, 3});
        REQUIRE(TestConstVector(ivec3, {1, 2, 3}));

        // Test construction of a 4-element int vector
        ConstValue ivec4 = ConstValue::CreateVector<int32_t>({1, 2, 3, 4});
        REQUIRE(TestConstVector(ivec4, {1, 2, 3, 4}));

        // Test construction of a 4-element float vector
        ConstValue vec4 = ConstValue::CreateVector<float>({1.0f, 2.0f, 3.0f, 4.0f});
        REQUIRE(TestConstVector(vec4, {1.0f, 2.0f, 3.0f, 4.0f}));
    }

    // Prepare some constants for later tests
    ConstValue b0    = ConstValue::CreateScalar(false);
    ConstValue b1    = ConstValue::CreateScalar(true);
    ConstValue i0    = ConstValue::CreateScalar(0);
    ConstValue i1    = ConstValue::CreateScalar(1);
    ConstValue i2    = ConstValue::CreateScalar(2);
    ConstValue i3    = ConstValue::CreateScalar(3);
    ConstValue i4    = ConstValue::CreateScalar(4);
    ConstValue f2_75 = ConstValue::CreateScalar(2.75f);
    ConstValue f3    = ConstValue::CreateScalar(3.0f);
    ConstValue f4    = ConstValue::CreateScalar(4.0f);

    ConstValue bv4_0101 = ConstValue::CreateVector<bool>({false, true, false, true});
    ConstValue bv4_0011 = ConstValue::CreateVector<bool>({false, false, true, true});
    ConstValue iv2_12   = ConstValue::CreateVector<int32_t>({1, 2});
    ConstValue iv2_45   = ConstValue::CreateVector<int32_t>({4, 5});
    ConstValue iv4_0123 = ConstValue::CreateVector<int32_t>({0, 1, 2, 3});
    ConstValue iv4_0024 = ConstValue::CreateVector<int32_t>({0, 0, 2, 4});
    ConstValue iv4_1111 = ConstValue::CreateVector<int32_t>({1, 1, 1, 1});
    ConstValue uv4_0123 = ConstValue::CreateVector<uint32_t>({0, 1, 2, 3});
    ConstValue v2_12    = ConstValue::CreateVector<float>({1.0f, 2.0f});
    ConstValue v3_123   = ConstValue::CreateVector<float>({1.0f, 2.0f, 3.0f});
    ConstValue v4_0123  = ConstValue::CreateVector<float>({0.0f, 1.0f, 2.0f, 3.0f});
    ConstValue v4_1111  = ConstValue::CreateVector<float>({1.0f, 1.0f, 1.0f, 1.0f});

    ConstValue im2x3_123456 =
        ConstValue::ConstructMatrix(ConstValue::CreateVector<int32_t>({1, 2, 3, 4, 5, 6}), ScalarKind::Int, 2, 3);
    ConstValue m2x3_123456 = ConstValue::ConstructMatrix(
        ConstValue::CreateVector<float>({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}), ScalarKind::Float, 2, 3);
    ConstValue m3x2_123456 = ConstValue::ConstructMatrix(
        ConstValue::CreateVector<float>({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}), ScalarKind::Float, 3, 2);

    SECTION("Unary Element-wise Arithmetics")
    {
        SECTION("Negate")
        {
            // Tests element-wise negation of scalars
            REQUIRE(TestConstScalar(i3.ElemwiseNegate(), int32_t(-3)));
            REQUIRE(TestConstScalar(f3.ElemwiseNegate(), -3.0f));

            // Element-wise negation of bools is not defined, should return error
            REQUIRE(b0.ElemwiseNegate().IsError());

            // Tests element-wise negation of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseNegate(), {0, -1, -2, -3}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseNegate(), {-0.0f, -1.0f, -2.0f, -3.0f}));
        }

        SECTION("BitNot")
        {
            // Tests element-wise bitwise-not of scalar integers
            REQUIRE(TestConstScalar(i3.ElemwiseBitNot(), int32_t(~3)));

            // Element-wise bitwise-not of bools and floats is not defined, should return error
            REQUIRE(b0.ElemwiseBitNot().IsError());
            REQUIRE(f3.ElemwiseBitNot().IsError());

            // Tests element-wise bitwise-not of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitNot(), {~0, ~1, ~2, ~3}));
        }

        SECTION("LogicalNot")
        {
            // Tests element-wise logical-not of bool scalars
            REQUIRE(TestConstScalar(b0.ElemwiseLogicalNot(), true));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalNot(), false));

            // Tests element-wise logical-not of vectors
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalNot(), {true, false, true, false}));
        }
    }

    SECTION("Binary Element-wise Arithmetics")
    {
        SECTION("Plus")
        {
            // Tests element-wise addition of scalars
            REQUIRE(TestConstScalar(i3.ElemwisePlus(i4), int32_t(7)));
            REQUIRE(TestConstScalar(f3.ElemwisePlus(f4), 3.0f + 4.0f));

            // Element-wise addition of bools is not defined, should return error
            REQUIRE(b0.ElemwisePlus(b0).IsError());

            // Tests element-wise addition of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwisePlus(iv4_1111), {1, 2, 3, 4}));
            REQUIRE(TestConstVector(v4_0123.ElemwisePlus(v4_1111), {1.0f, 2.0f, 3.0f, 4.0f}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwisePlus(i1), {1, 2, 3, 4}));
            REQUIRE(TestConstVector(i1.ElemwisePlus(iv4_0123), {1, 2, 3, 4}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwisePlus(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwisePlus(v4_0123).IsError());
        }

        SECTION("Minus")
        {
            // Tests element-wise subtraction of scalars
            REQUIRE(TestConstScalar(i3.ElemwiseMinus(i4), int32_t(-1)));
            REQUIRE(TestConstScalar(f3.ElemwiseMinus(f4), 3.0f - 4.0f));

            // Element-wise subtraction of bools is not defined, should return error
            REQUIRE(b0.ElemwiseMinus(b0).IsError());

            // Tests element-wise subtraction of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseMinus(iv4_1111), {-1, 0, 1, 2}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseMinus(v4_1111), {-1.0f, 0.0f, 1.0f, 2.0f}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseMinus(i1), {-1, 0, 1, 2}));
            REQUIRE(TestConstVector(i1.ElemwiseMinus(iv4_0123), {1, 0, -1, -2}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseMinus(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseMinus(v4_0123).IsError());
        }

        SECTION("Multiply")
        {
            // Tests element-wise multiplication of scalars
            REQUIRE(TestConstScalar(i3.ElemwiseMul(i4), int32_t(12)));
            REQUIRE(TestConstScalar(f3.ElemwiseMul(f4), 3.0f * 4.0f));

            // Element-wise multiplication of bools is not defined, should return error
            REQUIRE(b0.ElemwiseMul(b0).IsError());

            // Tests element-wise multiplication of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseMul(iv4_1111), {0, 1, 2, 3}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseMul(v4_1111), {0.0f, 1.0f, 2.0f, 3.0f}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseMul(i3), {0, 3, 6, 9}));
            REQUIRE(TestConstVector(i3.ElemwiseMul(iv4_0123), {0, 3, 6, 9}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseMul(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseMul(v4_0123).IsError());
        }

        SECTION("Divide")
        {
            // Tests element-wise division of scalars
            REQUIRE(TestConstScalar(i3.ElemwiseDiv(i4), int32_t(0)));
            REQUIRE(TestConstScalar(f3.ElemwiseDiv(f4), 3.0f / 4.0f));

            // Element-wise division of bools is not defined, should return error
            REQUIRE(b0.ElemwiseDiv(b0).IsError());

            // Tests element-wise division of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseDiv(iv4_1111), {0, 1, 2, 3}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseDiv(v4_1111), {0.0f, 1.0f, 2.0f, 3.0f}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseDiv(i1), {0, 1, 2, 3}));
            REQUIRE(TestConstVector(i4.ElemwiseDiv(iv4_1111), {4, 4, 4, 4}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseDiv(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseDiv(v4_0123).IsError());
        }

        SECTION("Modulo")
        {
            // Tests element-wise modulo of scalar integers
            REQUIRE(TestConstScalar(i3.ElemwiseMod(i4), int32_t(3)));

            // Element-wise modulo of bools and floats is not defined, should return error
            REQUIRE(b0.ElemwiseMod(b0).IsError());
            REQUIRE(f3.ElemwiseMod(f4).IsError());

            // Tests element-wise modulo of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseMod(iv4_1111), {0, 0, 0, 0}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseMod(i1), {0, 0, 0, 0}));
            REQUIRE(TestConstVector(i3.ElemwiseMod(iv4_1111), {0, 0, 0, 0}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseMod(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseMod(v4_0123).IsError());
        }

        SECTION("BitAnd")
        {
            // Tests element-wise bitwise-and of scalar integers
            REQUIRE(TestConstScalar(i3.ElemwiseBitAnd(i4), int32_t(0)));

            // Element-wise bitwise-and of bools and floats is not defined, should return error
            REQUIRE(b0.ElemwiseBitAnd(b0).IsError());
            REQUIRE(f3.ElemwiseBitAnd(f4).IsError());

            // Tests element-wise bitwise-and of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitAnd(iv4_1111), {0, 1, 0, 1}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitAnd(i1), {0, 1, 0, 1}));
            REQUIRE(TestConstVector(i1.ElemwiseBitAnd(iv4_0123), {0, 1, 0, 1}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseBitAnd(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseBitAnd(v4_0123).IsError());
        }

        SECTION("BitOr")
        {
            // Tests element-wise bitwise-or of scalar integers
            REQUIRE(TestConstScalar(i3.ElemwiseBitOr(i4), int32_t(7)));

            // Element-wise bitwise-or of bools and floats is not defined, should return error
            REQUIRE(b0.ElemwiseBitOr(b0).IsError());
            REQUIRE(f3.ElemwiseBitOr(f4).IsError());

            // Tests element-wise bitwise-or of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitOr(iv4_1111), {1, 1, 3, 3}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitOr(i1), {1, 1, 3, 3}));
            REQUIRE(TestConstVector(i1.ElemwiseBitOr(iv4_0123), {1, 1, 3, 3}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseBitOr(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseBitOr(v4_0123).IsError());
        }

        SECTION("BitXor")
        {
            // Tests element-wise bitwise-xor of scalar integers
            REQUIRE(TestConstScalar(i3.ElemwiseBitXor(i4), int32_t(7)));

            // Element-wise bitwise-xor of bools and floats is not defined, should return error
            REQUIRE(b0.ElemwiseBitXor(b0).IsError());
            REQUIRE(f3.ElemwiseBitXor(f4).IsError());

            // Tests element-wise bitwise-xor of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitXor(iv4_1111), {1, 0, 3, 2}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(iv4_0123.ElemwiseBitXor(i1), {1, 0, 3, 2}));
            REQUIRE(TestConstVector(i1.ElemwiseBitXor(iv4_0123), {1, 0, 3, 2}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseBitXor(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseBitXor(v4_0123).IsError());
        }

        SECTION("LogicalAnd")
        {
            // Tests element-wise logical-and of bool scalars
            REQUIRE(TestConstScalar(b0.ElemwiseLogicalAnd(b0), false));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalAnd(b0), false));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalAnd(b1), true));

            // Tests element-wise logical-and of vectors
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalAnd(bv4_0011), {false, false, false, true}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalAnd(b1), {false, true, false, true}));
            REQUIRE(TestConstVector(b1.ElemwiseLogicalAnd(bv4_0101), {false, true, false, true}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(bv4_0101.ElemwiseLogicalAnd(ConstValue::CreateVector<bool>({true, false})).IsError());
            REQUIRE(bv4_0101.ElemwiseLogicalAnd(iv4_0123).IsError());
        }

        SECTION("LogicalOr")
        {
            // Tests element-wise logical-or of bool scalars
            REQUIRE(TestConstScalar(b0.ElemwiseLogicalOr(b0), false));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalOr(b0), true));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalOr(b1), true));

            // Tests element-wise logical-or of vectors
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalOr(bv4_0011), {false, true, true, true}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalOr(b1), {true, true, true, true}));
            REQUIRE(TestConstVector(b0.ElemwiseLogicalOr(bv4_0101), {false, true, false, true}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(bv4_0101.ElemwiseLogicalOr(ConstValue::CreateVector<bool>({true, false})).IsError());
            REQUIRE(bv4_0101.ElemwiseLogicalOr(iv4_0123).IsError());
        }

        SECTION("LogicalXor")
        {
            // Tests element-wise logical-xor of bool scalars
            REQUIRE(TestConstScalar(b0.ElemwiseLogicalXor(b0), false));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalXor(b0), true));
            REQUIRE(TestConstScalar(b1.ElemwiseLogicalXor(b1), false));

            // Tests element-wise logical-xor of vectors
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalXor(bv4_0011), {false, true, true, false}));

            // Tests broadcasting of scalar to vector
            REQUIRE(TestConstVector(bv4_0101.ElemwiseLogicalXor(b1), {true, false, true, false}));
            REQUIRE(TestConstVector(b0.ElemwiseLogicalXor(bv4_0101), {false, true, false, true}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(bv4_0101.ElemwiseLogicalXor(ConstValue::CreateVector<bool>({true, false})).IsError());
            REQUIRE(bv4_0101.ElemwiseLogicalXor(iv4_0123).IsError());
        }

        SECTION("Shift")
        {
            // FIXME: Shift operators are not implemented yet.
            REQUIRE(i1.ElemwiseShiftLeft(i1).IsError());
            REQUIRE(i1.ElemwiseShiftRight(i1).IsError());
            REQUIRE(iv4_0123.ElemwiseShiftLeft(iv4_1111).IsError());
            REQUIRE(iv4_0123.ElemwiseShiftRight(iv4_1111).IsError());
        }
    }

    SECTION("Comparison")
    {
        SECTION("Equals")
        {
            // Tests element-wise equality of scalars
            REQUIRE(TestConstScalar(i3.ElemwiseEquals(i3), true));
            REQUIRE(TestConstScalar(i3.ElemwiseEquals(i4), false));
            REQUIRE(TestConstScalar(f3.ElemwiseEquals(f3), true));
            REQUIRE(TestConstScalar(b0.ElemwiseEquals(b1), false));

            // Tests element-wise equality of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseEquals(iv4_0024), {true, false, true, false}));
            REQUIRE(TestConstVector(bv4_0101.ElemwiseEquals(bv4_0011), {true, false, false, true}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseEquals(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseEquals(v4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseEquals(i1).IsError());
        }

        SECTION("NotEquals")
        {
            // Tests element-wise inequality of scalars
            REQUIRE(TestConstScalar(i3.ElemwiseNotEquals(i3), false));
            REQUIRE(TestConstScalar(i3.ElemwiseNotEquals(i4), true));
            REQUIRE(TestConstScalar(f3.ElemwiseNotEquals(f4), true));
            REQUIRE(TestConstScalar(b0.ElemwiseNotEquals(b1), true));

            // Tests element-wise inequality of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseNotEquals(iv4_0024), {false, true, false, true}));
            REQUIRE(TestConstVector(bv4_0101.ElemwiseNotEquals(bv4_0011), {false, true, true, false}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseNotEquals(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseNotEquals(v4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseNotEquals(i1).IsError());
        }

        SECTION("LessThan")
        {
            // Tests element-wise less-than of numeric scalars
            REQUIRE(TestConstScalar(i3.ElemwiseLessThan(i4), true));
            REQUIRE(TestConstScalar(i4.ElemwiseLessThan(i3), false));
            REQUIRE(TestConstScalar(f3.ElemwiseLessThan(f4), true));

            // Element-wise less-than of bools is not defined, should return error
            REQUIRE(b0.ElemwiseLessThan(b1).IsError());

            // Tests element-wise less-than of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseLessThan(iv4_0024), {false, false, false, true}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseLessThan(v4_1111), {true, false, false, false}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseLessThan(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseLessThan(v4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseLessThan(i1).IsError());
        }

        SECTION("LessThanEq")
        {
            // Tests element-wise less-than-or-equal of numeric scalars
            REQUIRE(TestConstScalar(i3.ElemwiseLessThanEq(i4), true));
            REQUIRE(TestConstScalar(i4.ElemwiseLessThanEq(i3), false));
            REQUIRE(TestConstScalar(f3.ElemwiseLessThanEq(f3), true));

            // Element-wise less-than-or-equal of bools is not defined, should return error
            REQUIRE(b0.ElemwiseLessThanEq(b1).IsError());

            // Tests element-wise less-than-or-equal of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseLessThanEq(iv4_0024), {true, false, true, true}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseLessThanEq(v4_1111), {true, true, false, false}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseLessThanEq(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseLessThanEq(v4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseLessThanEq(i1).IsError());
        }

        SECTION("GreaterThan")
        {
            // Tests element-wise greater-than of numeric scalars
            REQUIRE(TestConstScalar(i3.ElemwiseGreaterThan(i4), false));
            REQUIRE(TestConstScalar(i4.ElemwiseGreaterThan(i3), true));
            REQUIRE(TestConstScalar(f4.ElemwiseGreaterThan(f3), true));

            // Element-wise greater-than of bools is not defined, should return error
            REQUIRE(b1.ElemwiseGreaterThan(b0).IsError());

            // Tests element-wise greater-than of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseGreaterThan(iv4_0024), {false, true, false, false}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseGreaterThan(v4_1111), {false, false, true, true}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseGreaterThan(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseGreaterThan(v4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseGreaterThan(i1).IsError());
        }

        SECTION("GreaterThanEq")
        {
            // Tests element-wise greater-than-or-equal of numeric scalars
            REQUIRE(TestConstScalar(i3.ElemwiseGreaterThanEq(i4), false));
            REQUIRE(TestConstScalar(i4.ElemwiseGreaterThanEq(i3), true));
            REQUIRE(TestConstScalar(f3.ElemwiseGreaterThanEq(f3), true));

            // Element-wise greater-than-or-equal of bools is not defined, should return error
            REQUIRE(b1.ElemwiseGreaterThanEq(b0).IsError());

            // Tests element-wise greater-than-or-equal of vectors
            REQUIRE(TestConstVector(iv4_0123.ElemwiseGreaterThanEq(iv4_0024), {true, true, true, false}));
            REQUIRE(TestConstVector(v4_0123.ElemwiseGreaterThanEq(v4_1111), {false, true, true, true}));

            // Incompatible shapes or scalar types should return error
            REQUIRE(iv2_12.ElemwiseGreaterThanEq(iv4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseGreaterThanEq(v4_0123).IsError());
            REQUIRE(iv4_0123.ElemwiseGreaterThanEq(i1).IsError());
        }
    }

    SECTION("CastScalar")
    {
        SECTION("Scalar")
        {
            REQUIRE(i2.CastScalar(ScalarKind::Int) == ConstValue::CreateScalar(int32_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Bool), true));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Uint), uint32_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Float), 2.0f));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Double), 2.0));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Int8), int8_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Uint8), uint8_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Int16), int16_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Uint16), uint16_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Int64), int64_t(2)));
            REQUIRE(TestConstScalar(i2.CastScalar(ScalarKind::Uint64), uint64_t(2)));

            REQUIRE(TestConstScalar(i0.CastScalar(ScalarKind::Bool), false));
            REQUIRE(TestConstScalar(f2_75.CastScalar(ScalarKind::Int), int32_t(2)));
            REQUIRE(TestConstScalar(b1.CastScalar(ScalarKind::Float), 1.0f));
        }

        SECTION("Vector")
        {
            REQUIRE(TestConstVector(iv4_0123.CastScalar(ScalarKind::Bool), {false, true, true, true}));
            REQUIRE(TestConstVector(iv4_0123.CastScalar(ScalarKind::Float), {0.0f, 1.0f, 2.0f, 3.0f}));
            REQUIRE(TestConstVector(iv4_0123.CastScalar(ScalarKind::Double), {0.0, 1.0, 2.0, 3.0}));
            REQUIRE(TestConstVector(iv4_0123.CastScalar(ScalarKind::Uint64),
                                    {uint64_t(0), uint64_t(1), uint64_t(2), uint64_t(3)}));

            REQUIRE(TestConstVector(bv4_0101.CastScalar(ScalarKind::Int), {0, 1, 0, 1}));
        }

        SECTION("Matrix")
        {
            REQUIRE(TestConstMatrix(im2x3_123456.CastScalar(ScalarKind::Float), 2, 3,
                                    {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}));
            REQUIRE(
                TestConstMatrix(im2x3_123456.CastScalar(ScalarKind::Bool), 2, 3, {true, true, true, true, true, true}));
        }

        SECTION("Error")
        {
            REQUIRE(ConstValue{}.CastScalar(ScalarKind::Int).IsError());
            REQUIRE(i1.CastScalar(static_cast<ScalarKind>(999)).IsError());
        }
    }

    SECTION("GetElement")
    {
        REQUIRE(ConstValue{}.GetElement(0).IsError());

        REQUIRE(i3.GetElement(-1).IsError());
        REQUIRE(i3.GetElement(0) == ConstValue::CreateScalar(int32_t(3)));
        REQUIRE(i3.GetElement(1).IsError());

        REQUIRE(uv4_0123.GetElement(-1) == ConstValue{});
        REQUIRE(uv4_0123.GetElement(0) == ConstValue::CreateScalar(uint32_t(0)));
        REQUIRE(uv4_0123.GetElement(1) == ConstValue::CreateScalar(uint32_t(1)));
        REQUIRE(uv4_0123.GetElement(2) == ConstValue::CreateScalar(uint32_t(2)));
        REQUIRE(uv4_0123.GetElement(3) == ConstValue::CreateScalar(uint32_t(3)));
        REQUIRE(uv4_0123.GetElement(4) == ConstValue{});

        REQUIRE(TestConstVector(m2x3_123456.GetElement(0), {1.0f, 2.0f, 3.0f}));
        REQUIRE(TestConstVector(m2x3_123456.GetElement(1), {4.0f, 5.0f, 6.0f}));
        REQUIRE(m2x3_123456.GetElement(-1).IsError());
        REQUIRE(m2x3_123456.GetElement(2).IsError());
    }

    SECTION("GetSwizzle")
    {
        REQUIRE(ConstValue{}.GetSwizzle(SwizzleDesc::Parse("x")).IsError());

        REQUIRE(i3.GetSwizzle(SwizzleDesc::Parse("x")) == ConstValue::CreateScalar(int32_t(3)));
        REQUIRE(TestConstVector(i3.GetSwizzle(SwizzleDesc::Parse("xxxx")), {3, 3, 3, 3}));
        REQUIRE(i3.GetSwizzle(SwizzleDesc::Parse("y")).IsError());

        REQUIRE(iv2_45.GetSwizzle(SwizzleDesc::Parse("x")) == ConstValue::CreateScalar(int32_t(4)));
        REQUIRE(iv2_45.GetSwizzle(SwizzleDesc::Parse("y")) == ConstValue::CreateScalar(int32_t(5)));
        REQUIRE(TestConstVector(iv2_45.GetSwizzle(SwizzleDesc::Parse("yx")), {5, 4}));
        REQUIRE(TestConstVector(iv2_45.GetSwizzle(SwizzleDesc::Parse("xxxx")), {4, 4, 4, 4}));
        REQUIRE(iv2_45.GetSwizzle(SwizzleDesc::Parse("z")).IsError());

        REQUIRE(TestConstVector(v3_123.GetSwizzle(SwizzleDesc::Parse("xyz")), {1.0f, 2.0f, 3.0f}));
        REQUIRE(TestConstVector(v3_123.GetSwizzle(SwizzleDesc::Parse("zyx")), {3.0f, 2.0f, 1.0f}));
        REQUIRE(TestConstVector(v3_123.GetSwizzle(SwizzleDesc::Parse("rrg")), {1.0f, 1.0f, 2.0f}));
        REQUIRE(TestConstVector(v3_123.GetSwizzle(SwizzleDesc::Parse("stp")), {1.0f, 2.0f, 3.0f}));
        REQUIRE(v3_123.GetSwizzle(SwizzleDesc::Parse("w")).IsError());

        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("x")) == ConstValue::CreateScalar(uint32_t(0)));
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("xyzw")) == ConstValue::CreateVector<uint32_t>({0, 1, 2, 3}));
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("wzyx")) == ConstValue::CreateVector<uint32_t>({3, 2, 1, 0}));
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("xxzz")) == ConstValue::CreateVector<uint32_t>({0, 0, 2, 2}));
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("yy")) == ConstValue::CreateVector<uint32_t>({1, 1}));

        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("")).IsError());
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("xyzwx")).IsError());
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("xr")).IsError());
        REQUIRE(uv4_0123.GetSwizzle(SwizzleDesc::Parse("bad")).IsError());
    }

    SECTION("Length")
    {
        REQUIRE(ConstValue{}.Length().IsError());
        REQUIRE(TestConstScalar(i3.Length(), int32_t(1)));
        REQUIRE(TestConstScalar(v2_12.Length(), int32_t(2)));
        REQUIRE(TestConstScalar(v4_0123.Length(), int32_t(4)));
        REQUIRE(TestConstScalar(m2x3_123456.Length(), int32_t(3)));
        REQUIRE(TestConstScalar(m3x2_123456.Length(), int32_t(2)));
    }

    // FIXME: add more tests
}
