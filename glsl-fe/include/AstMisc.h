#pragma once
#include "AstBase.h"
#include <span>

namespace glsld
{
    // array sizes
    class MSVC_EMPTY_BASES AstArraySpec : public AstNodeBase, public AstPayload<AstArraySpec>
    {
    public:
        AstArraySpec()
        {
        }
        AstArraySpec(std::vector<AstExpr*> sizes) : sizes(std::move(sizes))
        {
        }

        auto GetSizeList() -> ArrayView<AstExpr*>
        {
            return sizes;
        }

        template <typename Visitor>
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

    struct LayoutItem
    {
        SyntaxToken idToken;
        AstExpr* value;
    };

    class MSVC_EMPTY_BASES AstLayoutQualifier : public AstNodeBase, public AstPayload<AstLayoutQualifier>
    {
    public:
        AstLayoutQualifier(std::vector<LayoutItem> items) : items(items)
        {
        }

        auto GetLayoutItems() -> ArrayView<LayoutItem>
        {
            return items;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            // FIXME: add dump
            return "";
        }

    private:
        std::vector<LayoutItem> items;
    };

    class QualifierGroup
    {
    public:
        auto CanDeclInterfaceBlock() -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

        auto HasStorageQual() -> bool
        {
            return qConst || qIn || qOut || qInout || qAttribute || qUniform || qVarying || qBuffer || qShared;
        }

        auto CanDeclareInterfaceBlock() -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

        auto GetHighp() -> bool
        {
            return qHighp;
        }
        auto SetHighp() -> void
        {
            qHighp = true;
        }
        auto GetMediump() -> bool
        {
            return qMediump;
        }
        auto SetMediump() -> void
        {
            qMediump = true;
        }
        auto GetLowp() -> bool
        {
            return qLowp;
        }
        auto SetLowp() -> void
        {
            qLowp = true;
        }

        auto SetConst() -> void
        {
            qConst = true;
        }
        auto GetConst() -> bool
        {
            return qConst;
        }
        auto SetIn() -> void
        {
            qIn = true;
        }
        auto GetIn() -> bool
        {
            return qIn;
        }
        auto SetOut() -> void
        {
            qOut = true;
        }
        auto GetOut() -> bool
        {
            return qOut;
        }
        auto SetInout() -> void
        {
            qInout = true;
        }
        auto GetInout() -> bool
        {
            return qInout;
        }
        auto SetAttribute() -> void
        {
            qAttribute = true;
        }
        auto GetAttribute() -> bool
        {
            return qAttribute;
        }
        auto SetUniform() -> void
        {
            qUniform = true;
        }
        auto GetUniform() -> bool
        {
            return qUniform;
        }
        auto SetVarying() -> void
        {
            qVarying = true;
        }
        auto GetVarying() -> bool
        {
            return qVarying;
        }
        auto SetBuffer() -> void
        {
            qBuffer = true;
        }
        auto GetBuffer() -> bool
        {
            return qBuffer;
        }
        auto SetShared() -> void
        {
            qShared = true;
        }
        auto GetShared() -> bool
        {
            return qShared;
        }

        auto SetCentroid() -> void
        {
            qCentroid = true;
        }
        auto GetCentroid() -> bool
        {
            return qCentroid;
        };
        auto SetSample() -> void
        {
            qSample = true;
        }
        auto GetSample() -> bool
        {
            return qSample;
        };
        auto SetPatch() -> void
        {
            qPatch = true;
        }
        auto GetPatch() -> bool
        {
            return qPatch;
        };

    public:
        // Precision Qualifier
        bool qHighp : 1   = false;
        bool qMediump : 1 = false;
        bool qLowp : 1    = false;

        // Storage/Parameter qualifiers
        bool qConst : 1     = false;
        bool qIn : 1        = false;
        bool qOut : 1       = false;
        bool qInout : 1     = false;
        bool qAttribute : 1 = false;
        bool qUniform : 1   = false;
        bool qVarying : 1   = false;
        bool qBuffer : 1    = false;
        bool qShared : 1    = false;

        // Auxiliary storage qualifiers
        bool qCentroid : 1 = false;
        bool qSample : 1   = false;
        bool qPatch : 1    = false;

        // Interpolation qualifiers
        bool qSmooth : 1        = false;
        bool qFlat : 1          = false;
        bool qNoperspective : 1 = false;

        // Variance qualifier
        bool qInvariant : 1 = false;

        // Precise qualifier
        bool qPrecise : 1 = false;

        // Memory qualifiers
        bool qCoherent : 1  = false;
        bool qVolatile : 1  = false;
        bool qRestrict : 1  = false;
        bool qReadonly : 1  = false;
        bool qWriteonly : 1 = false;
    };

    class MSVC_EMPTY_BASES AstTypeQualifierSeq : public AstNodeBase, public AstPayload<AstTypeQualifierSeq>
    {
    public:
        AstTypeQualifierSeq(QualifierGroup quals, std::vector<AstLayoutQualifier*> layoutQuals)
            : quals(quals), layoutQuals(layoutQuals)
        {
        }

        auto GetQualfierGroup() const -> const QualifierGroup&
        {
            return quals;
        }
        auto GetLayoutQuals() const -> ArrayView<AstLayoutQualifier*>
        {
            return layoutQuals;
        }

        template <typename Visitor>
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
        std::vector<AstLayoutQualifier*> layoutQuals;
    };

    class MSVC_EMPTY_BASES AstQualType : public AstNodeBase, public AstPayload<AstQualType>
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

        auto GetQualifiers() -> AstTypeQualifierSeq*
        {
            return qualifiers;
        }
        auto GetArraySpec() -> AstArraySpec*
        {
            return arraySpec;
        }
        auto GetTypeNameTok() -> SyntaxToken
        {
            return typeName;
        }
        auto GetStructDecl() -> AstStructDecl*
        {
            return structDecl;
        }

        template <typename Visitor>
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
                return fmt::format("TypeName: {}", typeName.text.StrView());
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