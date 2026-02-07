#include "Language/ConstValue.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

using namespace glsld;

TEST_CASE("ConstValue")
{
    SECTION("CreateScalar")
    {
        // Test construction from bool
        ConstValue boolValue = ConstValue::CreateScalar(true);
        REQUIRE(boolValue.IsScalarBool());
        REQUIRE(boolValue.GetBufferAs<bool>()[0] == true);

        // Test construction from int32_t
        ConstValue intValue = ConstValue::CreateScalar(static_cast<int32_t>(42));
        REQUIRE(intValue.IsScalarInt32());
        REQUIRE(intValue.GetBufferAs<int32_t>()[0] == 42);

        // Test construction from uint32_t
        ConstValue uintValue = ConstValue::CreateScalar(static_cast<uint32_t>(42));
        REQUIRE(uintValue.IsScalarUInt32());
        REQUIRE(uintValue.GetBufferAs<uint32_t>()[0] == 42u);

        // Test construction from float
        ConstValue floatValue = ConstValue::CreateScalar(3.14f);
        REQUIRE(floatValue.IsScalarFloat());
        REQUIRE(floatValue.GetBufferAs<float>()[0] == Catch::Approx(3.14f));

        // Test construction from double
        ConstValue doubleValue = ConstValue::CreateScalar(2.718);
        REQUIRE(doubleValue.IsScalarDouble());
        REQUIRE(doubleValue.GetBufferAs<double>()[0] == Catch::Approx(2.718));

        // Test construction from int8_t
        ConstValue int8Value = ConstValue::CreateScalar(int8_t(42));
        REQUIRE(int8Value.IsScalarInt8());
        REQUIRE(int8Value.GetBufferAs<int8_t>()[0] == 42);

        // Test construction from uint8_t
        ConstValue uint8Value = ConstValue::CreateScalar(uint8_t(42));
        REQUIRE(uint8Value.IsScalarUInt8());
        REQUIRE(uint8Value.GetBufferAs<uint8_t>()[0] == 42u);

        // Test construction from int16_t
        ConstValue int16Value = ConstValue::CreateScalar(int16_t(42));
        REQUIRE(int16Value.IsScalarInt16());
        REQUIRE(int16Value.GetBufferAs<int16_t>()[0] == 42);

        // Test construction from uint16_t
        ConstValue uint16Value = ConstValue::CreateScalar(uint16_t(42));
        REQUIRE(uint16Value.IsScalarUInt16());
        REQUIRE(uint16Value.GetBufferAs<uint16_t>()[0] == 42u);

        // Test construction from int64_t
        ConstValue int64Value = ConstValue::CreateScalar(int64_t(42));
        REQUIRE(int64Value.IsScalarInt64());
        REQUIRE(int64Value.GetBufferAs<int64_t>()[0] == 42);

        // Test construction from uint64_t
        ConstValue uint64Value = ConstValue::CreateScalar(uint64_t(42));
        REQUIRE(uint64Value.IsScalarUInt64());
        REQUIRE(uint64Value.GetBufferAs<uint64_t>()[0] == 42u);
    }

    SECTION("CreateVector")
    {
        // Test construction of a 2-element int vector
        ConstValue ivec2 = ConstValue::CreateVector<int32_t>({1, 2});
        REQUIRE(ivec2.GetScalarKind() == ScalarKind::Int);
        REQUIRE(ivec2.GetRowSize() == 1);
        REQUIRE(ivec2.GetColumnSize() == 2);
        REQUIRE(std::ranges::equal(ivec2.GetBufferAs<int32_t>(), std::initializer_list<int32_t>{1, 2}));

        // Test construction of a 3-element int vector
        ConstValue ivec3 = ConstValue::CreateVector<int32_t>({1, 2, 3});
        REQUIRE(ivec3.GetScalarKind() == ScalarKind::Int);
        REQUIRE(ivec3.GetRowSize() == 1);
        REQUIRE(ivec3.GetColumnSize() == 3);
        REQUIRE(std::ranges::equal(ivec3.GetBufferAs<int32_t>(), std::initializer_list<int32_t>{1, 2, 3}));

        // Test construction of a 4-element int vector
        ConstValue ivec4 = ConstValue::CreateVector<int32_t>({1, 2, 3, 4});
        REQUIRE(ivec4.GetScalarKind() == ScalarKind::Int);
        REQUIRE(ivec4.GetRowSize() == 1);
        REQUIRE(ivec4.GetColumnSize() == 4);
        REQUIRE(std::ranges::equal(ivec4.GetBufferAs<int32_t>(), std::initializer_list<int32_t>{1, 2, 3, 4}));

        // Test construction of a 4-element float vector
        ConstValue vec4 = ConstValue::CreateVector<float>({1.0f, 2.0f, 3.0f, 4.0f});
        REQUIRE(vec4.GetScalarKind() == ScalarKind::Float);
        REQUIRE(vec4.GetRowSize() == 1);
        REQUIRE(vec4.GetColumnSize() == 4);
        REQUIRE(std::ranges::equal(vec4.GetBufferAs<float>(), std::initializer_list<float>{1.0f, 2.0f, 3.0f, 4.0f}));
    }

    SECTION("ScalarArithmetics")
    {
        // Test ElemwisePlus
        ConstValue b0 = ConstValue::CreateScalar(false);
        ConstValue i3 = ConstValue::CreateScalar(3);
        ConstValue i4 = ConstValue::CreateScalar(4);
        ConstValue f3 = ConstValue::CreateScalar(3.0f);
        ConstValue f4 = ConstValue::CreateScalar(4.0f);

        SECTION("Plus")
        {
            ConstValue result = i3.ElemwisePlus(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == 7);

            result = f3.ElemwisePlus(f4);
            REQUIRE(result.IsScalarFloat());
            REQUIRE(result.GetBufferAs<float>()[0] == Catch::Approx(3.0f + 4.0f));
        }

        SECTION("Minus")
        {
            ConstValue result = i3.ElemwiseMinus(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == -1);

            result = f3.ElemwiseMinus(f4);
            REQUIRE(result.IsScalarFloat());
            REQUIRE(result.GetBufferAs<float>()[0] == Catch::Approx(3.0f - 4.0f));
        }

        SECTION("Multiply")
        {
            ConstValue result = i3.ElemwiseMul(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == 12);

            result = f3.ElemwiseMul(f4);
            REQUIRE(result.IsScalarFloat());
            REQUIRE(result.GetBufferAs<float>()[0] == Catch::Approx(3.0f * 4.0f));
        }

        SECTION("Divide")
        {
            ConstValue result = i3.ElemwiseDiv(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == 0);

            result = f3.ElemwiseDiv(f4);
            REQUIRE(result.IsScalarFloat());
            REQUIRE(result.GetBufferAs<float>()[0] == Catch::Approx(3.0f / 4.0f));
        }

        SECTION("Modulo")
        {
            ConstValue result = i3.ElemwiseMod(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == 3);

            result = f3.ElemwiseMod(f4);
            REQUIRE(result.IsError());
        }

        SECTION("BitAnd")
        {
            ConstValue result = i3.ElemwiseBitAnd(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == 0);
        }

        SECTION("BitOr")
        {
            ConstValue result = i3.ElemwiseBitOr(i4);
            REQUIRE(result.IsScalarInt32());
            REQUIRE(result.GetBufferAs<int32_t>()[0] == 7);
        }
    }

    SECTION("Comparison")
    {
        ConstValue lhs    = ConstValue::CreateScalar(5);
        ConstValue rhs    = ConstValue::CreateScalar(5);
        ConstValue result = lhs.ElemwiseEquals(rhs);
        REQUIRE(result.IsScalarBool());
        REQUIRE(result.GetBoolValue() == true);

        rhs    = ConstValue::CreateScalar(3);
        result = lhs.ElemwiseGreaterThan(rhs);
        REQUIRE(result.GetBoolValue() == true);
    }

    SECTION("GetElement")
    {
        ConstValue uvec4 = ConstValue::CreateVector<uint32_t>({0, 1, 2, 3});

        REQUIRE(uvec4.GetElement(-1) == ConstValue{});
        REQUIRE(uvec4.GetElement(0) == ConstValue::CreateScalar(uint32_t(0)));
        REQUIRE(uvec4.GetElement(1) == ConstValue::CreateScalar(uint32_t(1)));
        REQUIRE(uvec4.GetElement(2) == ConstValue::CreateScalar(uint32_t(2)));
        REQUIRE(uvec4.GetElement(3) == ConstValue::CreateScalar(uint32_t(3)));
        REQUIRE(uvec4.GetElement(4) == ConstValue{});
    }

    SECTION("GetSwizzle")
    {
        ConstValue uvec4 = ConstValue::CreateVector<uint32_t>({0, 1, 2, 3});

        REQUIRE(uvec4.GetSwizzle(SwizzleDesc::Parse("xyzw")) == ConstValue::CreateVector<uint32_t>({0, 1, 2, 3}));
        REQUIRE(uvec4.GetSwizzle(SwizzleDesc::Parse("wzyx")) == ConstValue::CreateVector<uint32_t>({3, 2, 1, 0}));
        REQUIRE(uvec4.GetSwizzle(SwizzleDesc::Parse("xxzz")) == ConstValue::CreateVector<uint32_t>({0, 0, 2, 2}));
        REQUIRE(uvec4.GetSwizzle(SwizzleDesc::Parse("yy")) == ConstValue::CreateVector<uint32_t>({1, 1}));
    }

    // FIXME: add more tests
}
