#include "Support/StringView.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <string_view>
#include <vector>

using namespace glsld;

using namespace std::literals;

TEST_CASE("Support::StringViewTest")
{
    SECTION("Construction and basic accessors")
    {
        constexpr StringView empty;
        static_assert(empty.empty());
        static_assert(empty.size() == 0);

        constexpr StringView fromLiteral = "hello";
        static_assert(fromLiteral.size() == 5);
        static_assert(fromLiteral.front() == 'h');
        static_assert(fromLiteral.back() == 'o');
        static_assert(fromLiteral[1] == 'e');

        const char rawText[]      = "prefix-value-suffix";
        StringView fromPointer    = rawText;
        StringView fromCount      = {rawText + 7, 5};
        StringView fromStdView    = std::string_view{"prefix"};
        std::string backingString = "backing";
        StringView fromString{backingString};

        std::array<char, 5> arrayRange = {'a', 'l', 'p', 'h', 'a'};
        StringView fromRange{arrayRange};

        std::vector<char> vectorRange = {'v', 'i', 'e', 'w'};
        StringView fromIterators{vectorRange.begin() + 1, vectorRange.end()};

        CHECK(fromPointer == "prefix-value-suffix");
        CHECK(fromCount == "value");
        CHECK(fromStdView == "prefix");
        CHECK(fromString == "backing");
        CHECK(fromRange == "alpha");
        CHECK(fromIterators == "iew");
        CHECK(fromPointer.StdStrView() == "prefix-value-suffix"sv);
        CHECK(fromCount.Str() == "value");
        CHECK(static_cast<std::string>(fromStdView) == "prefix");
        CHECK(fromString.data() == backingString.data());
    }

    SECTION("Split by character and substring")
    {
        StringView view = "path/to/file.ext";

        auto [head, tail] = view.Split('/');
        CHECK(head == "path");
        CHECK(tail == "to/file.ext");

        auto [name, ext] = view.Split("/to/");
        CHECK(name == "path");
        CHECK(ext == "file.ext");

        auto [missingLeft, missingRight] = view.Split(':');
        CHECK(missingLeft == view);
        CHECK(missingRight.empty());

        auto [leadingLeft, leadingRight] = StringView{"::value"}.Split("::");
        CHECK(leadingLeft.empty());
        CHECK(leadingRight == "value");
    }

    SECTION("Trim operations")
    {
        StringView padded = " \t\n  hello world\r\n ";

        CHECK(padded.TrimFront() == "hello world\r\n ");
        CHECK(padded.TrimBack() == " \t\n  hello world");
        CHECK(padded.Trim() == "hello world");
        CHECK(StringView{"\t \n\r"}.Trim().empty());
        CHECK(StringView{"already-trimmed"}.Trim() == "already-trimmed");
    }

    SECTION("Take and drop operations")
    {
        StringView view = "abcdef";

        CHECK(view.Take(0).empty());
        CHECK(view.Take(3) == "abc");
        CHECK(view.Drop(0) == view);
        CHECK(view.Drop(3) == "def");
        CHECK(view.TakeBack(2) == "ef");
        CHECK(view.TakeBack(view.size()) == view);
        CHECK(view.DropBack(2) == "abcd");
        CHECK(view.DropBack(0) == view);
        CHECK(view.Drop(2).data() == view.data() + 2);
        CHECK(view.Take(4).size() == 4);
    }

    SECTION("Predicate-based slicing")
    {
        StringView view = "token=42;tail";

        CHECK(view.TakeUntil([](char ch) { return ch == '='; }) == "token");
        CHECK(view.DropUntil([](char ch) { return ch == '='; }) == "=42;tail");
        CHECK(view.TakeUntil([](char ch) { return ch == '!'; }) == view);
        CHECK(view.DropUntil([](char ch) { return ch == '!'; }).empty());
        CHECK(view.TakeBackUntil([](char ch) { return ch == ';'; }) == ";tail");
        CHECK(view.DropBackUntil([](char ch) { return ch == ';'; }) == "token=42");
    }

    SECTION("Search predicates")
    {
        StringView view = "shader.vert";

        CHECK(view.Contains('.'));
        CHECK(view.Contains("der.v"));
        CHECK_FALSE(view.Contains('x'));
        CHECK_FALSE(view.Contains("frag"));

        CHECK(view.StartWith('s'));
        CHECK(view.StartWith("shader"));
        CHECK_FALSE(view.StartWith('h'));
        CHECK_FALSE(view.StartWith("vert"));

        CHECK(view.EndWith('t'));
        CHECK(view.EndWith(".vert"));
        CHECK_FALSE(view.EndWith('r'));
        CHECK_FALSE(view.EndWith("shader"));
    }

    SECTION("Comparison, hashing, iteration and interop")
    {
        StringView lhs = "alpha";
        StringView rhs = "beta";

        CHECK(lhs == StringView{"alpha"});
        CHECK(lhs != rhs);
        CHECK((lhs <=> rhs) == std::strong_ordering::less);
        CHECK(lhs.GetHashCode() == std::hash<std::string_view>{}(lhs.StdStrView()));

        std::string iterated;
        for (char ch : lhs) {
            iterated.push_back(ch);
        }
        CHECK(iterated == "alpha");

        std::string combined = "name=";
        combined += lhs;
        CHECK(combined == "name=alpha");
        CHECK(fmt::format("{}:{}", lhs, rhs) == "alpha:beta");
    }
}