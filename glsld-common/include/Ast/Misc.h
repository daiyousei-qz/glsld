#pragma once
#include "Ast/Base.h"
#include "Language/Semantic.h"

namespace glsld
{
    // Array sizes in "[x][y]" and .etc
    // Sizes are expressions, but they can also be empty for unsized array like "[]".
    class AstArraySpec : public AstNode
    {
    private:
        std::vector<AstExpr*> sizes;

    public:
        AstArraySpec()
        {
        }
        AstArraySpec(std::vector<AstExpr*> sizes) : sizes(std::move(sizes))
        {
        }

        auto GetSizeList() const -> ArrayView<const AstExpr*>
        {
            return sizes;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (auto expr : sizes) {
                if (expr && !visitor.Traverse(*expr)) {
                    return false;
                }
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            for (auto expr : sizes) {
                if (expr) {
                    d.DumpChildNode("Size", *expr);
                }
                else {
                    d.DumpAttribute("Size", "<Empty>");
                }
            }
        }
    };

    // "layout = xxx"
    struct LayoutItem
    {
        SyntaxToken idToken;
        /*NotNull*/ AstExpr* value;
    };

    class AstTypeQualifierSeq : public AstNode
    {
    private:
        QualifierGroup quals;
        std::vector<LayoutItem> layoutQuals;

    public:
        AstTypeQualifierSeq(QualifierGroup quals, std::vector<LayoutItem> layoutQuals)
            : quals(quals), layoutQuals(std::move(layoutQuals))
        {
        }

        auto GetQualGroup() const noexcept -> QualifierGroup
        {
            return quals;
        }
        auto GetLayoutQuals() const noexcept -> ArrayView<LayoutItem>
        {
            return layoutQuals;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (const auto& layoutItem : layoutQuals) {
                if (layoutItem.value && !visitor.Traverse(*layoutItem.value)) {
                    return false;
                }
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            d.DumpAttribute("QualGroup", "FIXME");
            for (const auto& layoutItem : layoutQuals) {
                d.DumpChildItem("LayoutQual", [&](Dumper& d) {
                    d.DumpAttribute("Key",
                                    layoutItem.idToken.IsIdentifier() ? layoutItem.idToken.text.Str() : "<Error>");
                    d.DumpChildNode("Value", *layoutItem.value);
                });
            }
        }
    };

    // Note this is a (qualifier, type) pair. They are grouped together just for convienience.
    // Different from C/C++, qualifiers in GLSL are not part of the type.
    class AstQualType : public AstNode
    {
    private:
        AstTypeQualifierSeq* qualifiers = nullptr;
        SyntaxToken typeName            = {};
        AstStructDecl* structDecl       = nullptr;
        AstArraySpec* arraySpec         = nullptr;

        // Payload:
        const Type* resolvedType = nullptr;

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

        auto SetResolvedType(const Type* type) -> void
        {
            this->resolvedType = type;
        }
        auto GetResolvedType() const -> const Type*
        {
            return resolvedType;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (qualifiers && !visitor.Traverse(*qualifiers)) {
                return false;
            }
            if (structDecl && !visitor.Traverse(*structDecl)) {
                return false;
            }
            if (arraySpec && !visitor.Traverse(*arraySpec)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            if (!structDecl) {
                d.DumpAttribute("TypeName", typeName.IsIdentifier() ? typeName.text.Str() : "<Error>");
            }
            if (qualifiers) {
                d.DumpChildNode("Qualifiers", *qualifiers);
            }
            if (structDecl) {
                d.DumpChildNode("StructDecl", *structDecl);
            }
            if (arraySpec) {
                d.DumpChildNode("ArraySpec", *arraySpec);
            }
        }
    };
} // namespace glsld