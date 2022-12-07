#pragma once
#include "SyntaxToken.h"
#include "AstPayload.h"
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
        auto GetNumToken() -> uint32_t
        {
            return range.endTokenIndex - range.startTokenIndex;
        }
        auto GetRange() -> SyntaxTokenRange
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

        // Invoke expression `f(*this, args...)` where `this` is dispatched into the real type.
        //
        // This is defined at Ast.h because of definition issue
        template <typename F, typename... Args>
        inline auto DispatchInvoke(F&& f, Args&&... args);

    private:
        friend class AstContext;

        // This should be correctly called by AstContext during the construction of the node
        auto Initialize(AstNodeTag tag, SyntaxTokenRange range) -> void
        {
            this->tag   = tag;
            this->range = range;
        }

        AstNodeTag tag         = AstNodeTag::Invalid;
        SyntaxTokenRange range = {};
    };

    // FIXME: is there a way to add constraint for this?
    template <typename VisitorType>
    concept AstVisitorT = true;
} // namespace glsld