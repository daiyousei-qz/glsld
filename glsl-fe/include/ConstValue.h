#pragma once
#include "Semantic.h"
#include "Typing.h"
#include "VecMat.h"

#include <array>
#include <cstdint>
#include <variant>

namespace glsld
{
    enum class ConstValueType
    {
        Error,
        Bool,
        Int32,
        UInt32,
        Float,
        Double,

        Vec3f,
        Vec4f,
    };

    template <ConstValueType Type>
    struct ConstValueTraits;

    template <>
    struct ConstValueTraits<ConstValueType::Error>
    {
        struct ErrorTag
        {
        };

        using CType = ErrorTag;
    };
    template <>
    struct ConstValueTraits<ConstValueType::Bool>
    {
        using CType = bool;
    };
    template <>
    struct ConstValueTraits<ConstValueType::Int32>
    {
        using CType = int32_t;
    };
    template <>
    struct ConstValueTraits<ConstValueType::UInt32>
    {
        using CType = uint32_t;
    };
    template <>
    struct ConstValueTraits<ConstValueType::Float>
    {
        using CType = float;
    };
    template <>
    struct ConstValueTraits<ConstValueType::Double>
    {
        using CType = double;
    };
    template <>
    struct ConstValueTraits<ConstValueType::Vec3f>
    {
        using CType = Vec<float, 3>;
    };
    template <>
    struct ConstValueTraits<ConstValueType::Vec4f>
    {
        using CType = Vec<float, 4>;
    };

    // FIXME:
    class ConstValue
    {
    public:
        ConstValue() = default;
        ConstValue(bool value) : valueType(ConstValueType::Bool), boolValue(value)
        {
        }
        ConstValue(int32_t value) : valueType(ConstValueType::Int32), intValue(value)
        {
        }
        ConstValue(uint32_t value) : valueType(ConstValueType::UInt32), uintValue(value)
        {
        }
        ConstValue(float value) : valueType(ConstValueType::Float), floatValue(value)
        {
        }
        ConstValue(double value) : valueType(ConstValueType::Double), doubleValue(value)
        {
        }

        auto IsErrorValue() const -> bool
        {
            return valueType == ConstValueType::Error;
        }
        auto GetValueType() const -> ConstValueType
        {
            return valueType;
        }

        auto GetBoolValue() const -> bool
        {
            GLSLD_ASSERT(valueType == ConstValueType::Bool);
            return boolValue;
        }

        auto GetIntValue() const -> int32_t
        {
            GLSLD_ASSERT(valueType == ConstValueType::Int32);
            return intValue;
        }

        auto GetUIntValue() const -> uint32_t
        {
            GLSLD_ASSERT(valueType == ConstValueType::Int32);
            return intValue;
        }

        auto GetFloatValue() const -> float
        {
            GLSLD_ASSERT(valueType == ConstValueType::Float);
            return floatValue;
        }

        auto GetDoubleValue() const -> double
        {
            GLSLD_ASSERT(valueType == ConstValueType::Double);
            return doubleValue;
        }

        template <ConstValueType Type>
        auto GetCValue() const
        {
            if constexpr (Type == ConstValueType::Error) {
                GLSLD_UNREACHABLE();
            }
            else if constexpr (Type == ConstValueType::Bool) {
                return boolValue;
            }
            else if constexpr (Type == ConstValueType::Int32) {
                return intValue;
            }
            else if constexpr (Type == ConstValueType::UInt32) {
                return uintValue;
            }
            else if constexpr (Type == ConstValueType::Float) {
                return floatValue;
            }
            else if constexpr (Type == ConstValueType::Double) {
                return doubleValue;
            }
            else {
                GLSLD_UNREACHABLE();
            }
        }

        auto ToString() const -> std::string
        {
            switch (valueType) {
            case ConstValueType::Error:
                return fmt::format("<error>");
            case ConstValueType::Bool:
                return fmt::format("{}", boolValue);
            case ConstValueType::Int32:
                return fmt::format("{}", intValue);
            case ConstValueType::UInt32:
                return fmt::format("{}", uintValue);
            case ConstValueType::Float:
                return fmt::format("{}", floatValue);
            case ConstValueType::Double:
                return fmt::format("{}", doubleValue);

            case ConstValueType::Vec3f:
                return "vec3?";
                // return fmt::format("{}", vec3fValue);
            case ConstValueType::Vec4f:
                return "vec4?";
                // return fmt::format("{}", vec4fValue);
            }

            GLSLD_UNREACHABLE();
        }

        auto GetTypeDesc() const -> const TypeDesc*
        {
            switch (valueType) {
            case ConstValueType::Error:
                return GetErrorTypeDesc();
            case ConstValueType::Bool:
                return GetBuiltinTypeDesc(BuiltinType::Ty_bool);
            case ConstValueType::Int32:
                return GetBuiltinTypeDesc(BuiltinType::Ty_int);
            case ConstValueType::UInt32:
                return GetBuiltinTypeDesc(BuiltinType::Ty_uint);
            case ConstValueType::Float:
                return GetBuiltinTypeDesc(BuiltinType::Ty_float);
            case ConstValueType::Double:
                return GetBuiltinTypeDesc(BuiltinType::Ty_double);

            case ConstValueType::Vec3f:
                return GetBuiltinTypeDesc(BuiltinType::Ty_vec3);
            case ConstValueType::Vec4f:
                return GetBuiltinTypeDesc(BuiltinType::Ty_vec4);
            }

            GLSLD_UNREACHABLE();
        }

    private:
        ConstValueType valueType = ConstValueType::Error;

        union
        {
            bool boolValue;
            int32_t intValue;
            uint32_t uintValue;

            float floatValue;
            double doubleValue;

            // FIXME: move to heap
            Vec<float, 3> vec3fValue;
            Vec<float, 4> vec4fValue;
        };
    };

    auto EvaluateUnaryOp(UnaryOp op, ConstValue operand) -> ConstValue;
    auto EvaluateBinaryOp(BinaryOp op, ConstValue lhs, ConstValue rhs) -> ConstValue;
    auto EvaluateSelectOp(ConstValue predicate, ConstValue ifBranchVal, ConstValue elseBranchVal) -> ConstValue;
} // namespace glsld