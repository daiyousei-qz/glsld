#pragma once
#include "Ast/Base.h"
#include "Language/Semantic.h"

namespace glsld
{
    // Represents a translation unit in the source program, which is the root node of the AST.
    class AstTranslationUnit : public AstNode
    {
    private:
        // [Node]
        // Global declarations in the source program.
        ArrayView</*NotNull*/ AstDecl*> globalDecls;

    public:
        AstTranslationUnit(ArrayView<AstDecl*> globalDecls) : globalDecls(std::move(globalDecls))
        {
        }

        auto GetGlobalDecls() const -> ArrayView<const AstDecl*>
        {
            return globalDecls;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            for (auto decl : globalDecls) {
                if (!visitor.Traverse(*decl)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            for (auto decl : globalDecls) {
                d.DumpChildNode("Decl", *decl);
            }
        }
    };

    // Represents array specifiers or sizes in "[x][y]" and .etc
    // Note this node is used in both type specifiers and array access.
    class AstArraySpec : public AstNode
    {
    private:
        // [Node]
        // Sizes are expressions, but they can also be nullptr for unsized array like "[]".
        ArrayView<AstExpr*> sizes;

    public:
        AstArraySpec(ArrayView<AstExpr*> sizes) : sizes(sizes)
        {
        }

        auto GetSizeList() const -> ArrayView<const AstExpr*>
        {
            return sizes;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            for (auto expr : sizes) {
                if (expr && !visitor.Traverse(*expr)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
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

    // Key-value pair in "layout(key = value)".
    struct LayoutItem
    {
        AstSyntaxToken idToken;

        // Could be `nullptr` if there's no value, like "layout(xxx)".
        AstExpr* value;
    };

    // Represents a sequence of qualifiers in the source program. Could be either:
    // - Qualifiers like "const", "inout", "layout", etc.
    // - Layout qualifiers like "layout(xxx = yyy)", "layout(xxx)", etc.
    class AstTypeQualifierSeq : public AstNode
    {
    private:
        QualifierGroup quals;
        ArrayView<LayoutItem> layoutQuals;

    public:
        AstTypeQualifierSeq(QualifierGroup quals, ArrayView<LayoutItem> layoutQuals)
            : quals(quals), layoutQuals(layoutQuals)
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
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            for (const auto& layoutItem : layoutQuals) {
                if (layoutItem.value && !visitor.Traverse(*layoutItem.value)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            d.DumpAttribute("QualGroup", "FIXME");
            for (const auto& layoutItem : layoutQuals) {
                d.DumpChildItem("LayoutQual", [&](Dumper& d) {
                    d.DumpAttribute("Key",
                                    layoutItem.idToken.IsIdentifier() ? layoutItem.idToken.text.Str() : "<Error>");
                    if (layoutItem.value) {
                        d.DumpChildNode("Value", *layoutItem.value);
                    }
                });
            }
        }
    };

    // Represents a (qualifier, type) pair. They are grouped together just for convienience.
    // Different from C/C++, qualifiers in GLSL are not part of the type but are part of the declaration.
    class AstQualType : public AstNode
    {
    private:
        // [Node]
        // Qualifiers like "const", "inout", "layout", etc.
        // Could be `nullptr` if there's no qualifier.
        AstTypeQualifierSeq* qualifiers = nullptr;

        // [Node]
        // Type specifier: struct definition that's immediately used as a type.
        // This takes precedence over `typeName`.
        AstStructDecl* structDecl = nullptr;

        // [Node]
        // Type specifier: type name like "int", "float", "vec3", etc. or a struct name.
        AstSyntaxToken typeName = {};

        // [Node]
        // Array size specifiers like "[x][y]" and .etc after the type specifier.
        AstArraySpec* arraySpec = nullptr;

        // [Payload]
        // Actual type deduced for this type.
        const Type* resolvedType = nullptr;

    public:
        AstQualType(AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl, AstArraySpec* arraySpec)
            : qualifiers(qualifiers), structDecl(structDecl), arraySpec(arraySpec)
        {
        }
        AstQualType(AstTypeQualifierSeq* qualifiers, AstSyntaxToken typeName, AstArraySpec* arraySpec)
            : qualifiers(qualifiers), typeName(typeName), arraySpec(arraySpec)
        {
        }

        auto GetQualifiers() const -> const AstTypeQualifierSeq*
        {
            return qualifiers;
        }
        auto GetArraySpec() const -> const AstArraySpec*
        {
            return arraySpec;
        }
        auto GetTypeNameTok() const -> AstSyntaxToken
        {
            return typeName;
        }
        auto GetStructDecl() const -> const AstStructDecl*
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
        auto DoTraverse(Visitor& visitor) const -> bool
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

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            if (qualifiers) {
                d.DumpChildNode("Qualifiers", *qualifiers);
            }
            if (structDecl) {
                d.DumpChildNode("StructDecl", *structDecl);
            }
            else {
                d.DumpAttribute("TypeName", !typeName.IsUnknown() ? typeName.text.StrView() : "<Error>");
            }
            if (arraySpec) {
                d.DumpChildNode("ArraySpec", *arraySpec);
            }
        }
    };
} // namespace glsld