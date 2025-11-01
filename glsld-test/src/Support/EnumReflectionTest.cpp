#include "Support/EnumReflection.h"

#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <vector>
#include <ranges>

using namespace glsld;

TEST_CASE("EnumReflectionTest")
{
    enum class SimpleTestEnum
    {
        FlagA = 0,
        FlagB = 1,
        FlagC = 2,
        FlagD = 3,
    };

    EnumBitFlags<SimpleTestEnum> flags;
    REQUIRE(flags.GetBits() == 0);
    flags.SetBit(SimpleTestEnum::FlagA);
    REQUIRE(flags.GetBits() == 1);
    flags.SetBit(SimpleTestEnum::FlagC);
    REQUIRE(flags.GetBits() == 5);
    REQUIRE(flags.TestBit(SimpleTestEnum::FlagA) == true);
    REQUIRE(flags.TestBit(SimpleTestEnum::FlagB) == false);
    REQUIRE(flags.TestBit(SimpleTestEnum::FlagC) == true);
    REQUIRE(flags.TestBit(SimpleTestEnum::FlagD) == false);

    // FIXME: Use c++23 range ctor
    std::vector<SimpleTestEnum> vec;
    std::ranges::copy(flags, std::back_inserter(vec));
    REQUIRE(vec == std::vector<SimpleTestEnum>{SimpleTestEnum::FlagA, SimpleTestEnum::FlagC});

    flags.ClearBit(SimpleTestEnum::FlagA);
    REQUIRE(flags.TestBit(SimpleTestEnum::FlagA) == false);

    flags.ClearAll();
    REQUIRE(flags.GetBits() == 0);
}