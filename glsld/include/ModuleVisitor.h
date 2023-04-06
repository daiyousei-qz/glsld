#pragma once
#include "LexContext.h"
#include "AstContext.h"
#include "AstVisitor.h"
#include "Compiler.h"

namespace glsld
{
    template <typename Derived>
    class ModuleVisitor : public AstVisitor<Derived>
    {
    public:
        ModuleVisitor(const CompilerObject& compileResult) : data(&compileResult)
        {
        }

        auto Traverse() -> void
        {
            AstVisitor<Derived>::TraverseAst(data->GetAstContext());
        }

        using AstVisitor<Derived>::Traverse;

    protected:
        auto GetLexContext() const -> const LexContext&
        {
            return data->GetLexContext();
        }
        auto GetAstContext() const -> const AstContext&
        {
            return data->GetAstContext();
        }

        auto NodeContainPosition(const AstNodeBase& node, TextPosition position) const -> bool
        {
            TextRange nodeRange = this->GetLexContext().LookupTextRange(node.GetRange());
            return nodeRange.Contains(position);
        }

        auto EnterIfContainsPosition(const AstNodeBase& node, TextPosition position) const -> AstVisitPolicy
        {
            if (NodeContainPosition(node, position)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }
        auto EnterIfOverlapRange(const AstNodeBase& node, TextRange range) const -> AstVisitPolicy
        {
            TextRange nodeRange = this->GetLexContext().LookupTextRange(node.GetRange());
            if (nodeRange.Overlaps(range)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

    private:
        const CompilerObject* data;
    };

} // namespace glsld