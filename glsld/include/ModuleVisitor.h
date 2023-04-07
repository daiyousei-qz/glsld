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
        ModuleVisitor(const CompilerObject& compilerObject)
            : lexContext(compilerObject.GetLexContext()), astContext(compilerObject.GetAstContext())
        {
        }
        ModuleVisitor(const CompiledPreamble& compiledPreamble)
            : lexContext(compiledPreamble.GetLexContext()), astContext(compiledPreamble.GetAstContext())
        {
        }

        auto Traverse() -> void
        {
            AstVisitor<Derived>::TraverseAst(astContext);
        }

        using AstVisitor<Derived>::Traverse;

    protected:
        auto GetLexContext() const -> const LexContext&
        {
            return lexContext;
        }
        auto GetAstContext() const -> const AstContext&
        {
            return astContext;
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
        const LexContext& lexContext;
        const AstContext& astContext;
    };

} // namespace glsld