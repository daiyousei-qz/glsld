#pragma once
#include "SyntaxToken.h"

#include <vector>

namespace glsld
{
    class AstExpr;

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

        static constexpr bool isLeafNode = false;
        static constexpr AstNodeTag tag  = AstNodeTag::Invalid;
        static constexpr int tagBegin    = 0;
        static constexpr int tagEnd      = 0;
    };

#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)                                                                                        \
    template <>                                                                                                        \
    struct AstNodeTrait<TYPE>                                                                                          \
    {                                                                                                                  \
        static constexpr bool isLeafNode = false;                                                                      \
        static constexpr AstNodeTag tag  = AstNodeTag::Invalid;                                                        \
        static constexpr int tagBegin    = detail::AstTagBeginBase_##TYPE + 1;                                         \
        static constexpr int tagEnd      = detail::AstTagEndBase_##TYPE;                                               \
    };
#define DECL_AST_TYPE(TYPE)                                                                                            \
    template <>                                                                                                        \
    struct AstNodeTrait<TYPE>                                                                                          \
    {                                                                                                                  \
        static constexpr bool isLeafNode = true;                                                                       \
        static constexpr AstNodeTag tag  = AstNodeTag::TYPE;                                                           \
        static constexpr int tagBegin    = static_cast<int>(AstNodeTag::TYPE);                                         \
        static constexpr int tagEnd      = static_cast<int>(AstNodeTag::TYPE) + 1;                                     \
    };
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    class AstNodeBase
    {
    public:
        virtual ~AstNodeBase()
        {
        }

        auto GetTag() -> AstNodeTag
        {
            return tag;
        }

        auto GetRange() -> SyntaxRange
        {
            return range;
        }

        template <typename AstType>
        inline auto Is() -> bool
        {
            if constexpr (AstNodeTrait<AstType>::isLeafNode) {
                return GetTag() == AstNodeTrait<AstType>::tag;
            }
            else {
                int tagValue = static_cast<int>(GetTag());
                return tagValue >= AstNodeTrait<AstType>::tagBegin && tagValue < AstNodeTrait<AstType>::tagEnd;
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
} // namespace glsld