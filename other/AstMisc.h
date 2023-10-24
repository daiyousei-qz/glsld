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
        auto Traverse(Visitor& visitor) const -> bool
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
        auto Traverse(Visitor& visitor) const -> bool
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
        auto Traverse(Visitor& visitor) const -> bool
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

} // namespace glsld