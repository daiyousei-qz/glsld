#pragma once
#include <nlohmann/json.hpp>

namespace glsld::lsp
{
    using JsonObject = nlohmann::json;

    template <typename T>
    struct JsonSerializer;

    class JsonFromObjectMapper;
    class JsonToObjectMapper;

    enum class JsonMappingMode
    {
        FromJson,
        ToJson,
    };

    template <JsonMappingMode Mode>
    struct JsonMappingTrait;

    template <>
    struct JsonMappingTrait<JsonMappingMode::FromJson>
    {
        using MapperType = JsonFromObjectMapper;

        template <typename T>
        using ReferenceType = T&;
    };
    template <>
    struct JsonMappingTrait<JsonMappingMode::ToJson>
    {
        using MapperType = JsonToObjectMapper;

        template <typename T>
        using ReferenceType = const T&;
    };

    template <JsonMappingMode Mode>
    using JsonObjectMapper = typename JsonMappingTrait<Mode>::MapperType;

    template <JsonMappingMode Mode, typename T>
    using JsonMappingRef = typename JsonMappingTrait<Mode>::template ReferenceType<T>;

    template <typename T>
    inline auto FromJson(const JsonObject& j, T& value) -> bool
    {
        return JsonSerializer<T>::FromJson(j, value);
    }
    template <typename T>
    inline auto ToJson(const T& value) -> JsonObject
    {
        return JsonSerializer<T>::ToJson(value);
    }

    inline auto ParseJson(std::string_view s) noexcept -> std::optional<JsonObject>
    {
        auto result = JsonObject::parse(s, nullptr, false, true);
        if (result.is_discarded()) {
            return std::nullopt;
        }

        return std::move(result);
    }

    // To serialize an object type T
    // template <JsonMappingMode M>
    // auto MapJson(JsonObjectMapper<M> mapper, JsonMappingRef<M, T> value) -> bool
    // {
    //     if (!mapper.Map("field", value.field)) {
    //         return false;
    //     }
    // }

    namespace detail
    {
        template <typename T>
        inline auto ObjectFromJson(const JsonObject& j, T& value) -> bool;
        template <typename T>
        inline auto ObjectToJson(const T& value) -> JsonObject;
    } // namespace detail

    template <typename T>
    struct JsonSerializer
    {
        static auto FromJson(const JsonObject& j, T& value) -> bool
        {
            return detail::ObjectFromJson(j, value);
        }
        static auto ToJson(const T& value) -> JsonObject
        {
            return detail::ObjectToJson(value);
        }
    };

    template <typename T>
        requires(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
    struct JsonSerializer<T>
    {
        static auto FromJson(const JsonObject& j, T& value) -> bool
        {
            if constexpr (std::is_same_v<T, bool>) {
                if (!j.is_boolean()) {
                    return false;
                }
            }
            else if constexpr (std::is_signed_v<T>) {
                if (!j.is_number_integer()) {
                    return false;
                }
            }
            else if constexpr (std::is_unsigned_v<T>) {
                if (!j.is_number_unsigned()) {
                    return false;
                }
            }
            else if constexpr (std::is_floating_point_v<T>) {
                if (!j.is_number_float()) {
                    return false;
                }
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                if (!j.is_string()) {
                    return false;
                }
            }

            j.get_to(value);
            return true;
        }

        static auto ToJson(const T& value) -> JsonObject
        {
            return value;
        }
    };

    template <typename T>
    struct JsonSerializer<std::optional<T>>
    {
        static auto FromJson(const JsonObject& j, std::optional<T>& value) -> bool
        {
            if (j.is_null()) {
                value = std::nullopt;
                return true;
            }

            T tmp;
            if (JsonSerializer<T>::FromJson(j, tmp)) {
                value = std::move(tmp);
                return true;
            }

            return false;
        }
        static auto ToJson(const std::optional<T>& value) -> JsonObject
        {
            if (!value) {
                return nlohmann::json{nullptr};
            }

            return JsonSerializer<T>::ToJson(*value);
        }
    };

    template <typename T>
    struct JsonSerializer<std::vector<T>>
    {
        static auto FromJson(const JsonObject& j, std::vector<T>& value) -> bool
        {
            if (!j.is_array()) {
                return false;
            }

            std::vector<T> result;
            for (const auto& jelem : j.get_ref<const nlohmann::json::array_t&>()) {
                T tmp;
                if (!JsonSerializer<T>::FromJson(jelem, tmp)) {
                    return false;
                }

                result.push_back(std::move(tmp));
            }

            value = std::move(result);
            return true;
        }
        static auto ToJson(const std::vector<T>& value) -> JsonObject
        {
            nlohmann::json::array_t jarray;
            for (const auto& elem : value) {
                jarray.push_back(JsonSerializer<T>::ToJson(elem));
            }

            return jarray;
        }
    };

    template <typename Mapper>
    class JsonObjectMapperScope
    {
    public:
        JsonObjectMapperScope(Mapper* mapper, const char* key) : mapper(mapper)
        {
            mapper->EnterObject(key);
        }
        ~JsonObjectMapperScope()
        {
            Exit();
        }

        JsonObjectMapperScope(const JsonObjectMapperScope&)                    = delete;
        auto operator=(const JsonObjectMapperScope&) -> JsonObjectMapperScope& = delete;

        JsonObjectMapperScope(JsonObjectMapperScope&& other)
        {
            *this = std::move(other);
        }
        auto operator=(JsonObjectMapperScope&& other) -> JsonObjectMapperScope&
        {
            mapper       = other.mapper;
            other.mapper = nullptr;
        }

        auto Exit() -> void
        {
            if (mapper) {
                mapper->ExitObject();
                mapper = nullptr;
            }
        }

    private:
        Mapper* mapper = nullptr;
    };

    class JsonFromObjectMapper
    {
    public:
        JsonFromObjectMapper(const JsonObject& root) : root(&root), traversalStack({&root})
        {
        }

        auto EnterObjectScoped(const char* key) -> JsonObjectMapperScope<JsonFromObjectMapper>
        {
            return JsonObjectMapperScope<JsonFromObjectMapper>{this, key};
        }
        auto EnterObject(const char* key) -> void
        {
            auto currentNode = GetCurrentNode();
            if (currentNode) {
                auto it = currentNode->find(key);
                if (it != currentNode->end()) {
                    traversalStack.push_back(&it.value());
                    return;
                }
            }

            traversalStack.push_back(nullptr);
        }
        auto ExitObject() -> void
        {
            traversalStack.pop_back();
            assert(!traversalStack.empty());
        }

        template <typename T>
        auto Map(const char* key, T& value) -> bool
        {
            // json[key] = JsonSerializer<T>::ToJson(value);
            auto currentNode = GetCurrentNode();
            if (!currentNode) {
                return false;
            }

            auto it = currentNode->find(key);
            if (it == currentNode->end()) {
                return false;
            }

            return JsonSerializer<T>::FromJson(it.value(), value);
        }

        // TODO: refactor redundant code
        template <typename T>
        auto Map(const char* key, std::optional<T>& value) -> bool
        {
            // json[key] = JsonSerializer<T>::ToJson(value);
            auto currentNode = GetCurrentNode();
            if (!currentNode) {
                return false;
            }

            auto it = currentNode->find(key);
            if (it == currentNode->end()) {
                return true;
            }

            return JsonSerializer<std::optional<T>>::FromJson(it.value(), value);
        }

    private:
        auto GetCurrentNode() -> const JsonObject*
        {
            return traversalStack.back();
        }

        const JsonObject* root;
        std::vector<const JsonObject*> traversalStack;
    };

    class JsonToObjectMapper
    {
    public:
        JsonToObjectMapper()
        {
            traversalStack.push_back(&root);
        }

        auto EnterObjectScoped(const char* key) -> JsonObjectMapperScope<JsonToObjectMapper>
        {
            return JsonObjectMapperScope<JsonToObjectMapper>{this, key};
        }
        auto EnterObject(const char* key) -> void
        {
            auto& currentNode = *GetCurrentNode();
            auto& nextNode    = currentNode[key];
            assert(nextNode.is_null());
            nextNode = JsonObject::object_t{};
            traversalStack.push_back(&nextNode);
        }
        auto ExitObject() -> void
        {
            traversalStack.pop_back();
            assert(!traversalStack.empty());
        }

        template <typename T>
        auto Map(const char* key, const T& value) -> bool
        {
            (*GetCurrentNode())[key] = JsonSerializer<T>::ToJson(value);
            return true;
        }

        auto ExportJson() -> JsonObject
        {
            assert(traversalStack.size() == 1);
            return std::move(root);
        }

    private:
        auto GetCurrentNode() -> JsonObject*
        {
            return traversalStack.back();
        }

        JsonObject root = JsonObject::object_t{};
        std::vector<JsonObject*> traversalStack;
    };

    namespace detail
    {
        template <typename T>
        auto ObjectFromJson(const JsonObject& j, T& value) -> bool
        {
            JsonFromObjectMapper mapper{j};
            MapJson(mapper, value);
            return true;
        }
        template <typename T>
        auto ObjectToJson(const T& value) -> JsonObject
        {
            JsonToObjectMapper mapper;
            MapJson(mapper, value);
            return mapper.ExportJson();
        }

    } // namespace detail
} // namespace glsld::lsp
