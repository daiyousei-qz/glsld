#pragma once
#include "Semantic.h"
#include "Typing.h"
#include "VecMat.h"

#include <array>
#include <cstdint>
#include <variant>
#include <type_traits>

namespace glsld
{
    template <BuiltinType Type>
    struct ConstValueTraits;

#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, ...)                                                                    \
    template <>                                                                                                        \
    struct ConstValueTraits<BuiltinType::Ty_##GLSL_TYPE>                                                               \
    {                                                                                                                  \
        using CPPType = CPP_TYPE;                                                                                      \
    };
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE

    // FIXME:
    class ConstValue
    {
        // struct ErrorTag
        // {
        // };

        using ErrorTag = int;

    public:
        ConstValue() = default;

        template <BuiltinType Type>
        static ConstValue FromValue(typename ConstValueTraits<Type>::CPPType value)
            requires(!std::is_void_v<typename ConstValueTraits<Type>::CPPType>)
        {
            ConstValue result;
            result.valueType = Type;

            if constexpr (false) {
                GLSLD_UNREACHABLE();
            }
#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, ...)                                                                    \
    else if constexpr (Type == BuiltinType::Ty_##GLSL_TYPE)                                                            \
    {                                                                                                                  \
        result.val_##GLSL_TYPE = value;                                                                                \
    }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
            else {
                GLSLD_UNREACHABLE();
            }

            return result;
        }

        auto IsErrorValue() const -> bool
        {
            return valueType == BuiltinType::Ty_void;
        }
        auto HasBoolValue() const -> bool
        {
            return valueType == BuiltinType::Ty_bool;
        }
        auto HasIntValue() const -> bool
        {
            return valueType == BuiltinType::Ty_int;
        }
        auto HasUintValue() const -> bool
        {
            return valueType == BuiltinType::Ty_uint;
        }
        auto GetValueType() const -> BuiltinType
        {
            return valueType;
        }

        auto GetBoolValue() const -> bool
        {
            GLSLD_ASSERT(valueType == BuiltinType::Ty_bool);
            return val_bool;
        }
        auto GetIntValue() const -> int32_t
        {
            GLSLD_ASSERT(valueType == BuiltinType::Ty_int);
            return val_int;
        }
        auto GetUIntValue() const -> uint32_t
        {
            GLSLD_ASSERT(valueType == BuiltinType::Ty_uint);
            return val_uint;
        }

        template <BuiltinType Type>
        auto GetCValue() const
        {
            if constexpr (false) {
                GLSLD_UNREACHABLE();
            }
#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, ...)                                                                    \
    else if constexpr (Type == BuiltinType::Ty_##GLSL_TYPE)                                                            \
    {                                                                                                                  \
        return val_##GLSL_TYPE;                                                                                        \
    }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
            else {
                GLSLD_UNREACHABLE();
            }
        }

        auto ToString() const -> std::string
        {
            switch (valueType) {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, ...)                                                                    \
    case BuiltinType::Ty_##GLSL_TYPE:                                                                                  \
        if constexpr (std::is_void_v<CPP_TYPE>) {                                                                      \
            return "<error>";                                                                                          \
        }                                                                                                              \
        else {                                                                                                         \
            return fmt::format("{}", val_##GLSL_TYPE);                                                                 \
        }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
            }

            GLSLD_UNREACHABLE();
        }

        auto GetTypeDesc() const -> const TypeDesc*
        {
            switch (valueType) {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, ...)                                                                    \
    case BuiltinType::Ty_##GLSL_TYPE:                                                                                  \
        if constexpr (std::is_void_v<CPP_TYPE>) {                                                                      \
            return GetErrorTypeDesc();                                                                                 \
        }                                                                                                              \
        else {                                                                                                         \
            return GetBuiltinTypeDesc(BuiltinType::Ty_##GLSL_TYPE);                                                    \
        }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
            }

            GLSLD_UNREACHABLE();
        }

    private:
        // We use void type as error
        BuiltinType valueType = BuiltinType::Ty_void;

        union
        {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, ...)                                                                    \
    std::conditional_t<std::is_void_v<CPP_TYPE>, ErrorTag, CPP_TYPE> val_##GLSL_TYPE;
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
        };
    };

    auto EvaluateUnaryOp(UnaryOp op, const ConstValue& operand) -> ConstValue;
    auto EvaluateBinaryOp(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue;
    auto EvaluateSelectOp(const ConstValue& predicate, const ConstValue& ifBranchVal, const ConstValue& elseBranchVal)
        -> ConstValue;
} // namespace glsld