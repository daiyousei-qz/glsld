#pragma once
#include "SyntaxToken.h"
#include "Semantic.h"

#include <vector>
#include <fmt/format.h>

namespace glsld
{
    class Type;

// Forward declaration of all Ast types
#define DECL_AST_BEGIN_BASE(TYPE) class TYPE;
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE) class TYPE;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    namespace detail
    {
        enum AstNodeTagValue
        {
            InvalidAstNodeTagValue = 0,
            AstNodeTagMin          = 0,

#define DECL_AST_BEGIN_BASE(TYPE) AstTagBeginBase_##TYPE,
#define DECL_AST_END_BASE(TYPE) AstTagEndBase_##TYPE,
#define DECL_AST_TYPE(TYPE) AstTagType_##TYPE,
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

            AstNodeTagMax,
        };

        // Maps AstNodeTagValue to actual Ast type. Invalid tag will be mapped to void.
        template <AstNodeTagValue TagValue>
        struct AstTypeLookupHelper
        {
            using Type = void;
        };

#define DECL_AST_BEGIN_BASE(TYPE)                                                                                      \
    template <>                                                                                                        \
    struct AstTypeLookupHelper<AstTagBeginBase_##TYPE>                                                                 \
    {                                                                                                                  \
        using Type = TYPE;                                                                                             \
    };
#define DECL_AST_END_BASE(TYPE)                                                                                        \
    template <>                                                                                                        \
    struct AstTypeLookupHelper<AstTagEndBase_##TYPE>                                                                   \
    {                                                                                                                  \
        using Type = TYPE;                                                                                             \
    };
#define DECL_AST_TYPE(TYPE)                                                                                            \
    template <>                                                                                                        \
    struct AstTypeLookupHelper<AstTagType_##TYPE>                                                                      \
    {                                                                                                                  \
        using Type = TYPE;                                                                                             \
    };
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

        template <AstNodeTagValue TagValue>
        using AstTypeOf = typename AstTypeLookupHelper<TagValue>::Type;

        // Utilities to tell information about the tag value
        //

        template <AstNodeTagValue TagValue>
        inline constexpr bool IsValidAstNodeTagValue = TagValue > AstNodeTagMin&& TagValue < AstNodeTagMax;

        template <AstNodeTagValue TagValue>
        inline constexpr bool IsBeginBaseAstNodeTagValue = false;

        template <AstNodeTagValue TagValue>
        inline constexpr bool IsEndBaseAstNodeTagValue = false;

        template <AstNodeTagValue TagValue>
        inline constexpr bool IsLeafAstNodeTagValue = false;

        template <AstNodeTagValue TagValue>
        inline constexpr bool IsBaseAstNodeTagValue =
            IsBeginBaseAstNodeTagValue<TagValue> || IsEndBaseAstNodeTagValue<TagValue>;

#define DECL_AST_BEGIN_BASE(TYPE)                                                                                      \
    template <>                                                                                                        \
    inline constexpr bool IsBeginBaseAstNodeTagValue<AstTagBeginBase_##TYPE> = true;
#define DECL_AST_END_BASE(TYPE)                                                                                        \
    template <>                                                                                                        \
    inline constexpr bool IsEndBaseAstNodeTagValue<AstTagEndBase_##TYPE> = true;
#define DECL_AST_TYPE(TYPE)                                                                                            \
    template <>                                                                                                        \
    inline constexpr bool IsLeafAstNodeTagValue<AstTagType_##TYPE> = true;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

        // Utilities to find parent type of an Ast type
        //

        template <AstNodeTagValue TagValue>
        inline constexpr AstNodeTagValue PrevAstNodeTagValue = static_cast<AstNodeTagValue>(TagValue - 1);

        template <AstNodeTagValue TagValue>
        inline constexpr AstNodeTagValue ParentAstNodeTagValue = InvalidAstNodeTagValue;

#define DECL_AST_BEGIN_BASE(TYPE)                                                                                      \
    template <>                                                                                                        \
    inline constexpr AstNodeTagValue ParentAstNodeTagValue<AstTagBeginBase_##TYPE> =                                   \
        IsBeginBaseAstNodeTagValue<PrevAstNodeTagValue<AstTagBeginBase_##TYPE>>                                        \
            ? PrevAstNodeTagValue<AstTagBeginBase_##TYPE>                                                              \
            : ParentAstNodeTagValue<PrevAstNodeTagValue<AstTagBeginBase_##TYPE>>;
#define DECL_AST_END_BASE(TYPE)                                                                                        \
    template <>                                                                                                        \
    inline constexpr AstNodeTagValue ParentAstNodeTagValue<AstTagEndBase_##TYPE> =                                     \
        ParentAstNodeTagValue<AstTagBeginBase_##TYPE>;
#define DECL_AST_TYPE(TYPE)                                                                                            \
    template <>                                                                                                        \
    inline constexpr AstNodeTagValue ParentAstNodeTagValue<AstTagType_##TYPE> =                                        \
        IsBeginBaseAstNodeTagValue<PrevAstNodeTagValue<AstTagType_##TYPE>>                                             \
            ? PrevAstNodeTagValue<AstTagType_##TYPE>                                                                   \
            : ParentAstNodeTagValue<PrevAstNodeTagValue<AstTagType_##TYPE>>;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
    } // namespace detail

    enum class AstNodeTag
    {
        Invalid = detail::InvalidAstNodeTagValue,

#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE) TYPE = detail::AstTagType_##TYPE,
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
    };

    auto AstNodeTagToString(AstNodeTag tag) -> StringView;

    template <typename AstType>
    struct AstNodeTrait
    {
        static_assert(AlwaysFalse<AstType>);

        // If AstType is a type for a leaf node
        static constexpr bool isLeafNode = false;

        // The tag for the AstType if it's a leaf node, otherwise it's Invalid
        static constexpr AstNodeTag tag = AstNodeTag::Invalid;

        // Any valid tag in range [tagBegin, tagEnd] is or derives AstType
        static constexpr int tagBegin = detail::InvalidAstNodeTagValue;
        static constexpr int tagEnd   = detail::InvalidAstNodeTagValue;

        using NodeType   = detail::AstTypeOf<detail::InvalidAstNodeTagValue>;
        using ParentType = detail::AstTypeOf<detail::InvalidAstNodeTagValue>;
    };

#define DECL_AST_BEGIN_BASE(TYPE)                                                                                      \
    template <>                                                                                                        \
    struct AstNodeTrait<TYPE>                                                                                          \
    {                                                                                                                  \
        static constexpr bool isLeafNode = false;                                                                      \
        static constexpr AstNodeTag tag  = AstNodeTag::Invalid;                                                        \
        static constexpr int tagBegin    = detail::AstTagBeginBase_##TYPE;                                             \
        static constexpr int tagEnd      = detail::AstTagEndBase_##TYPE;                                               \
        using NodeType                   = TYPE;                                                                       \
        using ParentType = detail::AstTypeOf<detail::ParentAstNodeTagValue<detail::AstTagBeginBase_##TYPE>>;           \
    };
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                             \
    template <>                                                                                                         \
    struct AstNodeTrait<TYPE>                                                                                           \
    {                                                                                                                   \
        static constexpr bool isLeafNode = true;                                                                        \
        static constexpr AstNodeTag tag  = AstNodeTag::TYPE;                                                            \
        static constexpr int tagBegin    = static_cast<int>(AstNodeTag::TYPE);                                          \
        static constexpr int tagEnd      = static_cast<int>(AstNodeTag::TYPE);                                          \
        using NodeType                   = TYPE;                                                                        \
        using ParentType                 = detail::AstTypeOf<detail::ParentAstNodeTagValue<detail::AstTagType_##TYPE>>; \
    };
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    // The base class of all Ast type
    class AstNode
    {
    public:
        AstNode() = default;

        AstNode(const AstNode&)                    = delete;
        auto operator=(const AstNode&) -> AstNode& = delete;

#if defined(GLSLD_DEBUG)
        // We don't need virtual dtor because we dispatch Ast type manually, but this is helpful for debugging
        virtual ~AstNode()
        {
        }
#endif

        auto GetTag() const noexcept -> AstNodeTag
        {
            return tag;
        }

        auto GetModuleId() const noexcept -> int
        {
            return moduleId;
        }

        // Number of tokens that this Ast node covers
        auto GetNumToken() const noexcept -> uint32_t
        {
            return range.endTokenIndex - range.startTokenIndex;
        }
        auto GetSyntaxRange() const noexcept -> AstSyntaxRange
        {
            return range;
        }
        auto GetFirstTokenIndex() const noexcept -> SyntaxTokenIndex
        {
            return range.startTokenIndex;
        }
        auto GetLastTokenIndex() const noexcept -> SyntaxTokenIndex
        {
            if (range.endTokenIndex == 0) {
                return 0;
            }
            else {
                return range.endTokenIndex - 1;
            }
        }

        template <typename AstType>
        auto Is() const -> bool
        {
            int tagValue = static_cast<int>(GetTag());
            return tagValue >= AstNodeTrait<AstType>::tagBegin && tagValue <= AstNodeTrait<AstType>::tagEnd;
        }

        template <typename AstType>
        auto As() -> AstType*
        {
            return Is<AstType>() ? static_cast<AstType*>(this) : nullptr;
        }

        auto Dump() -> void;

        // Invoke expression `f(*this, args...)` where `this` is dispatched into the real type.
        //
        // This is defined at Ast.h because of inheritance relation is unknown until the real
        // class is defined.
        template <typename F, typename... Args>
        auto DispatchInvoke(F&& f, Args&&... args);

    private:
        friend class AstContext;

        // This should be correctly called by AstContext during the construction of the node
        auto Initialize(AstNodeTag tag, int moduleId, AstSyntaxRange range) -> void
        {
            this->tag      = tag;
            this->moduleId = moduleId;
            this->range    = range;
        }

        // An integral tag that identifies the Ast type
        AstNodeTag tag = AstNodeTag::Invalid;

        // A unique id that identifies the parent module
        int moduleId = -1;

        // A range of ids that identifies syntax tokens that form this Ast node
        AstSyntaxRange range = {};
    };

    // An observing pointer into a declaration, including an index of declarator.
    // For declaration without declarators, that index should always be zero.
    class DeclView
    {
    public:
        DeclView() = default;
        DeclView(AstDecl* decl) : decl(decl)
        {
        }
        DeclView(AstDecl* decl, size_t index) : decl(decl), index(index)
        {
        }

        auto IsValid() const -> bool
        {
            return decl != nullptr;
        }

        auto GetDecl() const -> AstDecl*
        {
            return decl;
        }
        auto GetIndex() const -> size_t
        {
            return index;
        }

        operator bool() const
        {
            return IsValid();
        }

        auto operator==(const DeclView&) const -> bool = default;

    private:
        // Referenced declaration AST.
        AstDecl* decl = nullptr;

        // Declarator index. For declarations that cannot declare multiple symbols, this must be 0.
        size_t index = 0;
    };

    template <typename VisitorType>
    concept AstVisitorT = requires(VisitorType visitor, AstNode& astNode) {
        visitor.Traverse(astNode);
        visitor.Traverse(&astNode);
    };
} // namespace glsld