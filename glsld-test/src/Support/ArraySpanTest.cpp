#include "Support/ArraySpan.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <span>
#include <vector>

using namespace glsld;

TEST_CASE("Support::ArraySpanTest")
{
    SECTION("Construction and basic accessors")
    {
        constexpr ArraySpan<const int> empty;
        static_assert(empty.empty());
        static_assert(empty.size() == 0);

        int literalValues[]              = {1, 2, 3, 4};
        ArraySpan<const int> fromLiteral = literalValues;

        int rawValues[] = {3, 5, 8, 13, 21};
        ArraySpan<int> fromPointer{rawValues + 1, 3};

        std::array<int, 4> arrayValues = {2, 4, 6, 8};
        ArraySpan<int> fromStdArray{arrayValues};

        std::vector<int> vectorValues = {10, 20, 30, 40, 50};
        ArraySpan<int> fromRange{vectorValues};
        ArraySpan<int> fromIterators{vectorValues.begin() + 1, vectorValues.end() - 1};

        std::span<int> stdSpan = vectorValues;
        ArraySpan<int> fromStdSpan{stdSpan.subspan(2, 2)};

        ArraySpan<int, 4> fixedExtent{arrayValues};
        ArrayView<int> constView{arrayValues};

        CHECK(fromPointer.size() == 3);
        CHECK(fromPointer.front() == 5);
        CHECK(fromPointer.back() == 13);
        CHECK(fromPointer[1] == 8);
        CHECK(fromLiteral.size() == 4);
        CHECK(fromLiteral.front() == 1);
        CHECK(fromLiteral.back() == 4);
        CHECK(fromLiteral[1] == 2);
        CHECK(fromStdArray.StdSpan().data() == arrayValues.data());
        CHECK(fromStdArray.StdSpan().size() == arrayValues.size());
        CHECK(fromRange.data() == vectorValues.data());
        CHECK(fromIterators.size() == 3);
        CHECK(fromIterators.front() == 20);
        CHECK(fromIterators.back() == 40);
        CHECK(fromStdSpan == ArraySpan<int>{vectorValues.data() + 2, 2});
        CHECK(fixedExtent.size() == arrayValues.size());
        CHECK(constView.front() == 2);
        CHECK(constView.data() == arrayValues.data());
    }

    SECTION("Mutability tracks the underlying storage")
    {
        std::array<int, 4> values = {1, 2, 3, 4};
        ArraySpan<int> view{values};

        view.front() = 10;
        view.back()  = 40;
        view[1]      = 20;

        CHECK(values == std::array<int, 4>{10, 20, 3, 40});

        auto tail = view.Drop(2);
        tail[0]   = 30;

        CHECK(values == std::array<int, 4>{10, 20, 30, 40});
    }

    SECTION("Take and drop operations")
    {
        std::array<int, 6> values = {1, 2, 3, 4, 5, 6};
        ArraySpan<int> view{values};

        CHECK(view.Take(0).empty());
        CHECK(view.Take(3) == ArraySpan<int>{values.data(), 3});
        CHECK(view.Drop(0) == view);
        CHECK(view.Drop(3) == ArraySpan<int>{values.data() + 3, 3});
        CHECK(view.TakeBack(2) == ArraySpan<int>{values.data() + 4, 2});
        CHECK(view.TakeBack(view.size()) == view);
        CHECK(view.DropBack(2) == ArraySpan<int>{values.data(), 4});
        CHECK(view.DropBack(0) == view);
        CHECK(view.Drop(2).data() == view.data() + 2);
        CHECK(view.Take(4).size() == 4);
    }

    SECTION("Predicate-based slicing")
    {
        std::array<int, 7> values = {4, 8, 15, 16, 23, 42, 108};
        ArraySpan<int> view{values};

        CHECK(view.TakeWhile([](int value) { return value != 16; }) == ArraySpan<int>{values.data(), 3});
        CHECK(view.DropWhile([](int value) { return value != 16; }) == ArraySpan<int>{values.data() + 3, 4});
        CHECK(view.TakeWhile([](int value) { return value != 999; }) == view);
        CHECK(view.DropWhile([](int value) { return value != 999; }).empty());
        CHECK(view.TakeBackWhile([](int value) { return value != 23; }) == ArraySpan<int>{values.data() + 5, 2});
        CHECK(view.DropBackWhile([](int value) { return value != 23; }) == ArraySpan<int>{values.data(), 5});
        CHECK(view.TakeBackWhile([](int value) { return value != 999; }) == view);
        CHECK(view.DropBackWhile([](int value) { return value != 999; }).empty());
    }

    SECTION("Comparison and iteration")
    {
        std::array<int, 3> lhsValues = {1, 2, 3};
        std::array<int, 3> rhsValues = {1, 2, 4};

        ArraySpan<int> lhs{lhsValues};
        ArraySpan<int> rhs{rhsValues};
        ArraySpan<int> lhsCopy{lhsValues};

        CHECK(lhs == lhsCopy);
        CHECK(lhs != rhs);

        int sum = 0;
        for (int value : lhs) {
            sum += value;
        }
        CHECK(sum == 6);

        CHECK(*lhs.begin() == 1);
        CHECK(*(lhs.end() - 1) == 3);
    }

    SECTION("Const views interoperate with subspans")
    {
        std::vector<int> values = {7, 9, 11, 13, 15};

        ArrayView<int> view{values};
        auto middle = view.Drop(1).DropBack(1);

        CHECK(middle.size() == 3);
        CHECK(middle.front() == 9);
        CHECK(middle.back() == 13);
        CHECK(middle.StdSpan().data() == values.data() + 1);
        CHECK(middle.StdSpan().size() == 3);
    }
}