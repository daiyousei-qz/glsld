
#include "Compiler/CompilerObject.h"
#include "LanguageQueryProvider.h"
#include "Protocol.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

#include <vector>

namespace glsld
{
    class CollectReferenceVisitor : public LanguageQueryVisitor<CollectReferenceVisitor>
    {
    public:
        CollectReferenceVisitor(std::vector<lsp::Location>& output, const LanguageQueryProvider& provider,
                                lsp::DocumentUri uri, DeclView referenceDecl)
            : LanguageQueryVisitor(provider), documentUri(std::move(uri)), output(output), referenceDecl(referenceDecl)
        {
        }

        auto Execute() -> void
        {
            TraverseAllGlobalDecl();
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            if (expr.GetAccessedDecl() == referenceDecl) {
                AddReferenceToken(expr.GetAccessName());
            }
        }

        auto VisitAstQualType(AstQualType& qualType) -> void
        {
            if (!qualType.GetStructDecl() && qualType.GetResolvedStructDecl() == referenceDecl) {
                AddReferenceToken(qualType.GetTypeNameTok());
            }
        }

    private:
        auto AddReferenceToken(const SyntaxToken& token) -> void
        {
            // FIXME: Support reference from included files
            if (GetProvider().InMainFile(token)) {
                output.push_back(lsp::Location{
                    documentUri, ToLspRange(GetProvider().GetLexContext().LookupExpandedTextRange(token))});
            }
        }

        std::vector<lsp::Location>& output;

        lsp::DocumentUri documentUri;
        DeclView referenceDecl;
    };

    auto CollectDeclAsReferences(std::vector<lsp::Location>& output, const lsp::DocumentUri& uri, DeclView declView)
        -> void
    {
        auto& decl = *declView.GetDecl();
        if (auto funcDecl = decl.As<AstFunctionDecl>()) {
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
        }
        else if (auto memberDecl = decl.As<AstStructMemberDecl>()) {
        }
        else if (auto structDecl = decl.As<AstStructDecl>()) {
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
        }
    }

    // FIXME: Support includeDeclaration
    // FIXME: Support type references
    // FIXME: Support swizzle
    // FIXME: Reference in MACROs?
    auto ComputeReferences(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri, lsp::Position position,
                           bool includeDeclaration) -> std::vector<lsp::Location>
    {
        auto declTokInfo = provider.LookupSymbolAccess(FromLspPosition(position));
        if (!declTokInfo || !declTokInfo->symbolDecl.IsValid()) {
            return {};
        }

        std::vector<lsp::Location> result;
        if (includeDeclaration) {
            CollectDeclAsReferences(result, uri, declTokInfo->symbolDecl);
        }
        CollectReferenceVisitor{result, provider, uri, declTokInfo->symbolDecl}.Execute();
        return result;
    }
} // namespace glsld