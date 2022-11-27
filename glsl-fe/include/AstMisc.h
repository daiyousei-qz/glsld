#pragma once
#include "AstBase.h"

namespace glsld
{
    // array sizes
    class AstArraySpec : public AstNodeBase
    {
    public:
        AstArraySpec()
        {
        }
        AstArraySpec(std::vector<AstExpr*> sizes) : sizes(std::move(sizes))
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

    private:
        std::vector<AstExpr*> sizes;
    };

    class AstLayoutQualifier : public AstNodeBase
    {
    public:
        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }
    };
    class AstTypeQualifierSeq : public AstNodeBase
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

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

    protected:
        bool qHighp : 1   = false;
        bool qMediump : 1 = false;
        bool qLowp : 1    = false;

        // Storage qualifiers
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
    };

    class AstQualType : public AstNodeBase
    {
    public:
        AstQualType(AstTypeQualifierSeq* qualifiers, SyntaxToken typeName) : qualifiers(qualifiers), typeName(typeName)
        {
        }
        AstQualType(AstTypeQualifierSeq* qualifiers, AstStructDecl* structDecl)
            : qualifiers(qualifiers), structDecl(structDecl)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            // FIXME: how to traverse expressions in the layout qualifier?
        }

    private:
        AstTypeQualifierSeq* qualifiers = nullptr;
        AstArraySpec* arraySpec         = nullptr;

        SyntaxToken typeName      = {};
        AstStructDecl* structDecl = nullptr;
    };
} // namespace glsld