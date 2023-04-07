#pragma once
#include "AstImpl.h"
#include <algorithm>

namespace glsld
{
    // array sizes
    class AstArraySpec : public AstImpl<AstArraySpec>
    {
    public:
        AstArraySpec()
        {
        }
        AstArraySpec(std::vector<AstExpr*> sizes) : sizes(std::move(sizes))
        {
        }

        auto GetSizeList() const -> ArrayView<AstExpr*>
        {
            return sizes;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto expr : sizes) {
                visitor.Traverse(expr);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        std::vector<AstExpr*> sizes;
    };

    // A layout item encodes "identifier" or "identifier=expr" in the layout qualifier
    struct LayoutItem
    {
        SyntaxToken idToken;
        AstExpr* value;
    };

    class AstTypeQualifierSeq : public AstImpl<AstTypeQualifierSeq>
    {
    public:
        AstTypeQualifierSeq(QualifierGroup quals, std::vector<LayoutItem> layoutQuals)
            : quals(quals), layoutQuals(std::move(layoutQuals))
        {
        }

        auto GetQualfierGroup() const -> const QualifierGroup&
        {
            return quals;
        }
        auto GetLayoutQuals() const -> ArrayView<LayoutItem>
        {
            return layoutQuals;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            // TODO: add dump
            return "";
        }

    private:
        QualifierGroup quals;
        std::vector<LayoutItem> layoutQuals;
    };

    class AstQualType : public AstImpl<AstQualType>
    {
    public:
        AstQualType(AstTypeQualifierSeq* qualifiers, SyntaxToken typeName, AstArraySpec* arraySpec)
            : qualifiers(qualifiers), typeName(typeName), arraySpec(arraySpec)
        {
        }
        AstQualType(AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl, AstArraySpec* arraySpec)
            : qualifiers(qualifiers), structDecl(structDecl), arraySpec(arraySpec)
        {
        }

        auto GetQualifiers() const -> AstTypeQualifierSeq*
        {
            return qualifiers;
        }
        auto GetArraySpec() const -> AstArraySpec*
        {
            return arraySpec;
        }
        auto GetTypeNameTok() const -> SyntaxToken
        {
            return typeName;
        }
        auto GetStructDecl() const -> AstStructDecl*
        {
            return structDecl;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            // FIXME: how to traverse expressions in the layout qualifier?
            visitor.Traverse(qualifiers);
            visitor.Traverse(arraySpec);
            visitor.Traverse(structDecl);
        }

        auto DumpNodeData() const -> std::string
        {
            if (!structDecl) {
                return fmt::format("BaseTypeToken: {}", typeName.text.StrView());
            }
            else {
                return "";
            }
        }

    private:
        AstTypeQualifierSeq* qualifiers = nullptr;
        AstArraySpec* arraySpec         = nullptr;

        SyntaxToken typeName      = {};
        AstStructDecl* structDecl = nullptr;
    };

    // class AstDeclarator : public AstImpl<AstDeclarator>
    // {
    // public:
    //     auto GetDeclToken() -> SyntaxToken
    //     {
    //         return declTok;
    //     }
    //     auto GetArraySpec() -> AstArraySpec*
    //     {
    //         return arraySpec;
    //     }
    //     auto GetInitializer() -> AstExpr*
    //     {
    //         return initializer;
    //     }

    //     template <AstVisitorT Visitor>
    //     auto Traverse(Visitor& visitor) -> void
    //     {
    //         visitor.Traverse(arraySpec);
    //         visitor.Traverse(initializer);
    //     }

    //     auto DumpNodeData() const -> std::string
    //     {
    //         return fmt::format("DeclToken: {}", declTok.text.StrView());
    //     }

    // private:
    //     SyntaxToken declTok;
    //     AstArraySpec* arraySpec;
    //     AstExpr* initializer;
    // };

    class AstInitializerList final : public AstImpl<AstInitializerList>
    {
    public:
        AstInitializerList(std::vector<AstNodeBase*> items) : items(items)
        {
            GLSLD_ASSERT(std::ranges::all_of(
                items, [](AstNodeBase* node) { return node->Is<AstExpr>() || node->Is<AstInitializerList>(); }));
        }

        auto GetItems() const -> const std::vector<AstNodeBase*>&
        {
            return items;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto node : items) {
                visitor.Traverse(*node);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        std::vector<AstNodeBase*> items;
    };
} // namespace glsld