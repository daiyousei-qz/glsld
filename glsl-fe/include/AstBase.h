#pragma once
#include "SyntaxToken.h"
#include "Semantic.h"

#include <vector>
#include <fmt/format.h>

namespace glsld
{
#if defined(_MSC_VER)
#define MSVC_EMPTY_BASES __declspec(empty_bases)
#else
#define MSVC_EMPTY_BASES
#endif

    class TypeDesc;

    namespace detail
    {
        enum
        {
#define DECL_AST_BEGIN_BASE(TYPE) AstTagBeginBase_##TYPE,
#define DECL_AST_END_BASE(TYPE) AstTagEndBase_##TYPE,
#define DECL_AST_TYPE(TYPE) AstTagType_##TYPE,
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
        };
    } // namespace detail

    enum class AstNodeTag
    {
        Invalid,

#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE) TYPE = detail::AstTagType_##TYPE,
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
    };

    inline auto AstNodeTagToString(AstNodeTag tag) -> std::string_view
    {
        switch (tag) {
        case AstNodeTag::Invalid:
            return "Invalid";
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return #TYPE;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
        default:
            GLSLD_UNREACHABLE();
        }
    };

// Forward declaration of all Ast types
#define DECL_AST_BEGIN_BASE(TYPE) class TYPE;
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE) class TYPE;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    template <typename AstType>
    struct AstNodeTrait
    {
        // FIXME: static_assert(false);

        // If AstType is a type for a leaf node
        static constexpr bool isLeafNode = false;
        // The tag for the AstType if it's a leaf node, otherwise it's Invalid
        static constexpr AstNodeTag tag = AstNodeTag::Invalid;

        // any valid tag in range (tagBegin, tagEnd) is or derives AstType
        static constexpr int tagBegin = 0;
        static constexpr int tagEnd   = 0;
    };

#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)                                                                                        \
    template <>                                                                                                        \
    struct AstNodeTrait<TYPE>                                                                                          \
    {                                                                                                                  \
        static constexpr bool isLeafNode = false;                                                                      \
        static constexpr AstNodeTag tag  = AstNodeTag::Invalid;                                                        \
        static constexpr int tagBegin    = detail::AstTagBeginBase_##TYPE;                                             \
        static constexpr int tagEnd      = detail::AstTagEndBase_##TYPE;                                               \
    };
#define DECL_AST_TYPE(TYPE)                                                                                            \
    template <>                                                                                                        \
    struct AstNodeTrait<TYPE>                                                                                          \
    {                                                                                                                  \
        static constexpr bool isLeafNode = true;                                                                       \
        static constexpr AstNodeTag tag  = AstNodeTag::TYPE;                                                           \
        static constexpr int tagBegin    = static_cast<int>(AstNodeTag::TYPE) - 1;                                     \
        static constexpr int tagEnd      = static_cast<int>(AstNodeTag::TYPE) + 1;                                     \
    };
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    class AstNodeBase
    {
    public:
        AstNodeBase() = default;

        AstNodeBase(const AstNodeBase&)                    = delete;
        auto operator=(const AstNodeBase&) -> AstNodeBase& = delete;

#if defined(GLSLD_DEBUG)
        // We don't need virtual dtor because we dispatch Ast type manually, but this is helpful for debugging
        virtual ~AstNodeBase()
        {
        }
#endif

        auto GetTag() -> AstNodeTag
        {
            return tag;
        }

        // Number of tokens that this Ast node covers
        auto GetNumToken() -> int
        {
            return range.end.GetIndex() - range.begin.GetIndex();
        }
        auto GetRange() -> SyntaxRange
        {
            return range;
        }

        auto Dump() -> void;

        template <typename AstType>
        inline auto Is() -> bool
        {
            if constexpr (AstNodeTrait<AstType>::isLeafNode) {
                return GetTag() == AstNodeTrait<AstType>::tag;
            }
            else {
                int tagValue = static_cast<int>(GetTag());
                return tagValue > AstNodeTrait<AstType>::tagBegin && tagValue < AstNodeTrait<AstType>::tagEnd;
            }
        }

        template <typename AstType>
        inline auto As() -> AstType*
        {
            return Is<AstType>() ? static_cast<AstType*>(this) : nullptr;
        }

    private:
        friend class AstContext;

        // This should be correctly called by AstContext during the construction of the node
        auto Initialize(AstNodeTag tag, SyntaxRange range) -> void
        {
            this->tag   = tag;
            this->range = range;
        }

        AstNodeTag tag    = AstNodeTag::Invalid;
        SyntaxRange range = {};
    };

    template <typename AstType>
    class AstPayload
    {
    public:
    };

    template <>
    class AstPayload<AstQualType>
    {
    public:
    private:
        AstDecl* decl = nullptr;
    };

    template <>
    class AstPayload<AstExpr>
    {
    public:
        auto GetDeducedType() -> const TypeDesc*
        {
            return deducedType;
        }
        auto SetDeducedType(const TypeDesc* deducedType) -> void
        {
            this->deducedType = deducedType;
        }

        auto GetContextualType() -> const TypeDesc*
        {
            return contextualType;
        }
        auto SetContextualType(const TypeDesc* contextualType) -> void
        {
            this->contextualType = contextualType;
        }

    private:
        // Type of the evaluated expression
        const TypeDesc* deducedType = nullptr;

        // Type of the context where the expression is used
        const TypeDesc* contextualType = nullptr;
    };

    template <>
    class AstPayload<AstNameAccessExpr>
    {
    public:
        auto GetAccessType() -> NameAccessType
        {
            return accessType;
        }
        auto SetAccessType(NameAccessType accessType) -> void
        {
            this->accessType = accessType;
        }

        auto GetAccessedDecl() -> AstDecl*
        {
            return decl;
        }
        auto SetAccessedDecl(AstDecl* decl) -> void
        {
            this->decl = decl;
        }

    private:
        NameAccessType accessType = NameAccessType::Unknown;
        AstDecl* decl             = nullptr;
    };
} // namespace glsld