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
            : sourceContext(compilerObject.GetSourceContext()), lexContext(compilerObject.GetLexContext()),
              astContext(compilerObject.GetAstContext())
        {
        }
        ModuleVisitor(const CompiledPreamble& compiledPreamble)
            : sourceContext(compiledPreamble.GetSourceContext()), lexContext(compiledPreamble.GetLexContext()),
              astContext(compiledPreamble.GetAstContext())
        {
        }

        auto Traverse() -> void
        {
            AstVisitor<Derived>::TraverseAst(astContext);
        }

        using AstVisitor<Derived>::Traverse;

    protected:
        auto GetSourceContext() const -> const SourceContext&
        {
            return sourceContext;
        }

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
            TextRange nodeRange = GetLexContext().LookupExpandedTextRange(node.GetSyntaxRange());
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
            TextRange nodeRange = this->GetLexContext().LookupExpandedTextRange(node.GetSyntaxRange());
            if (nodeRange.Overlaps(range)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

    private:
        const SourceContext& sourceContext;
        const LexContext& lexContext;
        const AstContext& astContext;
    };

} // namespace glsld