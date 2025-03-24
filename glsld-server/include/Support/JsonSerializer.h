#pragma once
#include "SerializerUtils.h"

#include <boost/pfr.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>

#include <optional>
#include <type_traits>

namespace glsld
{
    template <typename T>
    struct JsonSerializer
    {
        static_assert(std::is_default_constructible_v<T>, "Type must be default constructible");
        static_assert(std::is_aggregate_v<T>, "Type must be an aggregate");

        static auto Serialize(const T& data) -> nlohmann::json
        {
            nlohmann::json j = nlohmann::json::object();
            boost::pfr::for_each_field(data, [&j]<size_t I>(const auto& field, std::integral_constant<size_t, I>) {
                using FieldType                 = std::decay_t<decltype(field)>;
                j[boost::pfr::get_name<I, T>()] = JsonSerializer<FieldType>::Serialize(field);
            });

            return j;
        }

        static auto Deserialize(T& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_object()) {
                return false;
            }

            bool result = true;
            boost::pfr::for_each_field(data, [&j, &result]<size_t I>(auto& field, std::integral_constant<size_t, I>) {
                using FieldType = std::decay_t<decltype(field)>;
                if (!result) {
                    return;
                }

                if (auto it = j.find(boost::pfr::get_name<I, T>()); it != j.end()) {
                    if (!JsonSerializer<FieldType>::Deserialize(field, *it)) {
                        result = false;
                    }
                }
            });

            return result;
        }
    };

    template <std::integral T>
    struct JsonSerializer<T>
    {
        static auto Serialize(const T& data) -> nlohmann::json
        {
            return data;
        }

        static auto Deserialize(T& data, const nlohmann::json& j) -> bool
        {
            if constexpr (std::is_signed_v<T>) {
                if (!j.is_number_integer()) {
                    return false;
                }
            }
            else {
                if (!j.is_number_unsigned()) {
                    return false;
                }
            }

            data = j.get<T>();
            return true;
        }
    };

    template <std::floating_point T>
    struct JsonSerializer<T>
    {
        static auto Serialize(const T& data) -> nlohmann::json
        {
            return data;
        }

        static auto Deserialize(T& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_number_float()) {
                return false;
            }

            data = j.get<T>();
            return true;
        }
    };

    template <typename T>
        requires std::is_enum_v<T>
    struct JsonSerializer<T>
    {
        static auto Serialize(const T& data) -> nlohmann::json
        {
            return static_cast<std::underlying_type_t<T>>(data);
        }

        static auto Deserialize(T& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_number_integer()) {
                return false;
            }

            if (auto result = magic_enum::enum_cast<T>(j.get<std::underlying_type_t<T>>())) {
                data = result.value();
            }
            else {
                return false;
            }

            return true;
        }
    };

    template <typename T>
    struct JsonSerializer<StringEnum<T>>
    {
        static auto Serialize(const StringEnum<T>& data) -> nlohmann::json
        {
            return magic_enum::enum_name(data.value);
        }

        static auto Deserialize(StringEnum<T>& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_string()) {
                return false;
            }

            if (auto result = magic_enum::enum_cast<T>(j.get<std::string_view>())) {
                data.value = result.value();
            }
            else {
                return false;
            }

            return true;
        }
    };

    template <>
    struct JsonSerializer<bool>
    {
        static auto Serialize(const bool& data) -> nlohmann::json
        {
            return data;
        }

        static auto Deserialize(bool& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_boolean()) {
                return false;
            }

            data = j.get<bool>();
            return true;
        }
    };

    template <>
    struct JsonSerializer<std::string>
    {
        static auto Serialize(const std::string& data) -> nlohmann::json
        {
            return data;
        }

        static auto Deserialize(std::string& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_string()) {
                return false;
            }

            data = j.get<std::string>();
            return true;
        }
    };

    template <typename T>
    struct JsonSerializer<std::optional<T>>
    {
        static auto Serialize(const std::optional<T>& data) -> nlohmann::json
        {
            if (!data) {
                return nlohmann::json(nullptr);
            }

            return JsonSerializer<T>::Serialize(*data);
        }

        static auto Deserialize(std::optional<T>& data, const nlohmann::json& j) -> bool
        {
            if (j.is_null()) {
                data = std::nullopt;
                return true;
            }

            T value;
            if (!JsonSerializer<T>::Deserialize(value, j)) {
                return false;
            }

            data = std::move(value);
            return true;
        }
    };

    template <typename T>
    struct JsonSerializer<std::vector<T>>
    {
        static auto Serialize(const std::vector<T>& data) -> nlohmann::json
        {
            nlohmann::json jarray = nlohmann::json::array();
            for (const auto& elem : data) {
                jarray.push_back(JsonSerializer<T>::Serialize(elem));
            }

            return jarray;
        }

        static auto Deserialize(std::vector<T>& data, const nlohmann::json& j) -> bool
        {
            if (!j.is_array()) {
                return false;
            }

            data.clear();
            for (const auto& jelem : j.get_ref<const nlohmann::json::array_t&>()) {
                T elem;
                if (!JsonSerializer<T>::Deserialize(elem, jelem)) {
                    return false;
                }

                data.push_back(std::move(elem));
            }

            return true;
        }
    };

    template <typename T>
    concept UserDefinedSerializableT = requires(T t, const nlohmann::json& j) {
        std::is_default_constructible_v<T>;
        { static_cast<const T&>(t).Serialize() } -> std::same_as<nlohmann::json>;
        { t.Deserialize(j) } -> std::same_as<bool>;
    };

    template <UserDefinedSerializableT T>
    struct JsonSerializer<T>
    {
        static auto Serialize(const T& data) -> nlohmann::json
        {
            return data.Serialize();
        }

        static auto Deserialize(T& data, const nlohmann::json& j) -> bool
        {
            return data.Deserialize(j);
        }
    };

} // namespace glsld