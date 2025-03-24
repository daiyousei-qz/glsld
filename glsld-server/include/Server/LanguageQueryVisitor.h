#pragma once
#include "Ast/AstVisitor.h"

#include "Server/LanguageQueryInfo.h"

namespace glsld
{
    template <typename Derived>
    class LanguageQueryVisitor : public AstVisitor<Derived>
    {
    private:
        const LanguageQueryInfo& info;

    public:
        LanguageQueryVisitor(const LanguageQueryInfo& info) : info(info)
        {
        }

        auto GetInfo() const -> const LanguageQueryInfo&
        {
            return info;
        }

    protected:
        auto TraverseTranslationUnit() -> void
        {
            this->Traverse(info.GetUserFileAst());
        }

        auto TraverseNodeContains(const AstNode& node, TextPosition position) -> AstVisitPolicy
        {
            if (GetInfo().ContainsPositionExtended(node, position)) {
                return AstVisitPolicy::Traverse;
            }
            else if (GetInfo().PrecedesPosition(node, position)) {
                return AstVisitPolicy::Leave;
            }
            else {
                return AstVisitPolicy::Halt;
            }
        }

        // auto TraverseGlobalDeclUntil(TextPosition position)
        // {
        //     for (AstDecl* decl : provider.GetAstContext().GetGlobalDecls()) {
        //         if (provider.SucceedsPosition(*decl, position)) {
        //             break;
        //         }

        //         this->Traverse(*decl);
        //     }
        // }

        // auto TraverseGlobalDeclOverlaps(TextRange range)
        // {
        //     // FIXME: halt early
        //     for (AstDecl* decl : provider.GetAstContext().GetGlobalDecls()) {
        //         TextRange declRange = provider.GetLexContext().LookupExpandedTextRange(decl->GetSyntaxRange());
        //         if (declRange.Overlaps(range)) {
        //             this->Traverse(*decl);
        //         }
        //     }
        // }
    };

} // namespace glsld