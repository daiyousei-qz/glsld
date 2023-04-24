#pragma once
#include "LexContext.h"
#include "AstContext.h"
#include "AstVisitor.h"
#include "Compiler.h"
#include "LanguageQueryProvider.h"

namespace glsld
{
    template <typename Derived>
    class ModuleVisitor : public AstVisitor<Derived>
    {
    public:
        ModuleVisitor(const LanguageQueryProvider& provider) : provider(provider)
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