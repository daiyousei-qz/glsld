#include "Support/JsonSerializer.h"

#include <catch2/catch_test_macros.hpp>

using namespace glsld;

TEST_CASE("JsonSerializerTest")
{
    auto testSerialization = [&](const auto& object) {
        using ObjectType = std::remove_cvref_t<decltype(object)>;
        auto json        = JsonSerializer<ObjectType>::Serialize(object);
        ObjectType deserializedObject;
        REQUIRE(JsonSerializer<ObjectType>::Deserialize(deserializedObject, json));
        REQUIRE(object == deserializedObject);
    };

    SECTION("Scalar")
    {
        testSerialization(true);
        testSerialization(42);
        testSerialization(3.14f);
        testSerialization(std::string{"Hello, World!"});
    }

    SECTION("Enum")
    {
        enum class TestEnum
        {
            Value1,
            Value2,
            Value3,
        };

        testSerialization(TestEnum::Value2);
        testSerialization(StringEnum<TestEnum>{TestEnum::Value2});
    }

    SECTION("Struct")
    {
        struct TestStruct
        {
            int a;
            float b;
            std::string c;

            auto operator==(const TestStruct& other) const -> bool = default;
        };

        testSerialization(TestStruct{42, 3.14f, "Hello"});
    }

    SECTION("Aggregate")
    {
        testSerialization(std::vector<int>{});
        testSerialization(std::vector<int>{1, 2, 3, 4, 5});
        testSerialization(std::optional<int>{});
        testSerialization(std::optional<int>{42});
    }

    SECTION("CustomType")
    {
        struct CustomType
        {
            std::string s = "";
            CustomType()  = default;
            CustomType(int x) : s(std::to_string(x))
            {
            }

            auto Serialize() const -> nlohmann::json
            {
                return nlohmann::json{{"s", std::stoi(s)}};
            }
            auto Deserialize(const nlohmann::json& json) -> bool
            {
                if (json.contains("s")) {
                    s = std::to_string(json.at("s").get<int>());
                    return true;
                }
                return false;
            }

            auto operator==(const CustomType& other) const -> bool = default;
        };

        testSerialization(CustomType{42});
    }
}