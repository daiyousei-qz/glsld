#pragma once
#include "Ast/AstVisitor.h"
#include "Compiler/LexContext.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerObject.h"

#include "LanguageQueryProvider.h"

namespace glsld
{
    template <typename Derived>
    class LanguageQueryVisitor : public AstVisitor<Derived>
    {
    public:
        LanguageQueryVisitor(const LanguageQueryProvider& provider) : provider(provider)
        {
        }

        auto GetProvider() const -> const LanguageQueryProvider&
        {
            return provider;
        }

    protected:
        auto TraverseAllGlobalDecl() -> void
        {
            for (AstDecl* decl : provider.GetAstContext().GetGlobalDecls()) {
                this->Traverse(*decl);
            }
        }

        auto TraverseGlobalDeclUntil(TextPosition position)
        {
            for (AstDecl* decl : provider.GetAstContext().GetGlobalDecls()) {
                if (provider.SucceedsPosition(*decl, position)) {
                    break;
                }

                this->Traverse(*decl);
            }
        }

        auto TraverseGlobalDeclOverlaps(TextRange range)
        {
            // FIXME: halt early
            for (AstDecl* decl : provider.GetAstContext().GetGlobalDecls()) {
                TextRange declRange = provider.GetLexContext().LookupExpandedTextRange(decl->GetSyntaxRange());
                if (declRange.Overlaps(range)) {
                    this->Traverse(*decl);
                }
            }
        }

    private:
        const LanguageQueryProvider& provider;
    };

} // namespace glsld