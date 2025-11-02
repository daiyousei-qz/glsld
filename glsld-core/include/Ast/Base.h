#pragma once
#include "Basic/Common.h"
#include "Support/StringView.h"
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    class Type;

// Forward declaration of all Ast types
#define DECL_AST_BEGIN_BASE(AST_TYPE) class AST_TYPE;
#define DECL_AST_END_BASE(AST_TYPE)
#define DECL_AST_TYPE(AST_TYPE) class AST_TYPE;
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

    template <typename AstType>
    struct AstNodeTrait
    {
        static_assert(AlwaysFalse<AstType> && "This is not a AST type in GlslAst.inc");

        // If AstType is a type for a leaf node
        static constexpr bool isLeafNode = false;

        // The tag for the AstType if it's a leaf node, otherwise it's Invalid
        static constexpr AstNodeTag tag = AstNodeTag::Invalid;

        // Any valid tag in range [tagBegin, tagEnd] is or derives AstType
        static constexpr int tagBegin = detail::InvalidAstNodeTagValue;
        static constexpr int tagEnd   = detail::InvalidAstNodeTagValue;

        static constexpr StringView name = "UnknownAstType";

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
        static constexpr StringView name = #TYPE;                                                                      \
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
        static constexpr StringView name = #TYPE;                                                                       \
        using NodeType                   = TYPE;                                                                        \
        using ParentType                 = detail::AstTypeOf<detail::ParentAstNodeTagValue<detail::AstTagType_##TYPE>>; \
    };
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    // The base class of all AST type.
    // An AST type contains the following information:
    // - A tag, which is used to identify the real type of this node.
    // - A syntax range, which is used to locate the node in the source code.
    // - Children, which is used to store the sub-nodes.
    // - Payloads, which is used to store additional information.
    class AstNode
    {
    private:
        // A tag to identify the type of this node. Each node type should have a unique tag.
        AstNodeTag tag;

        AstSyntaxRange range;

        friend class AstBuilder;

        // This must be called upon creation of a new AstNode.
        // Proper tag must be set as it is used to identify the type of this node.
        auto Initialize(AstNodeTag tag, AstSyntaxRange range) -> void
        {
            this->tag   = tag;
            this->range = range;
        }

    protected:
        AstNode() = default;

    public:
        AstNode(const AstNode&)            = delete;
        AstNode& operator=(const AstNode&) = delete;

#if defined(GLSLD_DEBUG)
        // Although we don't need virtual destructor, but this is still helpful for debugging.
        virtual ~AstNode() = default;
#endif

        auto GetTag() const noexcept -> AstNodeTag
        {
            return tag;
        }
        auto GetSyntaxRange() const noexcept -> AstSyntaxRange
        {
            return range;
        }

        template <typename AstType>
        auto Is() const noexcept -> bool
        {
            const int tagValue = static_cast<int>(tag);
            return tagValue >= AstNodeTrait<AstType>::tagBegin && tagValue <= AstNodeTrait<AstType>::tagEnd;
        }
        template <typename AstType>
        auto As() noexcept -> AstType*
        {
            return Is<AstType>() ? static_cast<AstType*>(this) : nullptr;
        }
        template <typename AstType>
        auto As() const noexcept -> const AstType*
        {
            return Is<AstType>() ? static_cast<const AstType*>(this) : nullptr;
        }

        auto ToString() const -> std::string;
        auto Dump() const -> void;
    };

    template <typename AstType>
    concept AstNodeT = AstNodeTrait<AstType>::tag != AstNodeTag::Invalid;

    template <typename AstType>
    concept AstLeafNodeT = AstNodeT<AstType> && AstNodeTrait<AstType>::isLeafNode;

    template <typename VisitorType>
    concept AstVisitorT = requires(VisitorType visitor, const AstNode& astNode) { visitor.Traverse(astNode); };

    template <typename PrinterType>
    concept AstPrinterT = requires(PrinterType printer, StringView key, const AstNode& astNode) {
        { printer.GetPointerIdentifier(static_cast<void*>(nullptr)) } -> std::convertible_to<uintptr_t>;
        printer.PrintAttribute(key, false);
        printer.PrintAttribute(key, 0);
        printer.PrintAttribute(key, "value");
        printer.PrintChildNode(key, astNode);
        printer.PrintChildItem(key, [](PrinterType & d) {});
    };
} // namespace glsld