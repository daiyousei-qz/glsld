
#include "Feature/Reference.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"

namespace glsld
{
    class CollectReferenceVisitor : public LanguageQueryVisitor<CollectReferenceVisitor>
    {
    private:
        std::vector<lsp::Location>& output;

        lsp::DocumentUri documentUri;

        DeclView referenceDecl;

        bool includeDeclaration;

    public:
        CollectReferenceVisitor(std::vector<lsp::Location>& output, const LanguageQueryInfo& info, lsp::DocumentUri uri,
                                DeclView referenceDecl, bool includeDeclaration)
            : LanguageQueryVisitor(info), documentUri(std::move(uri)), output(output), referenceDecl(referenceDecl),
              includeDeclaration(includeDeclaration)
        {
        }

        auto Execute() -> void
        {
            TraverseTranslationUnit();
        }

        auto VisitAstQualType(const AstQualType& qualType) -> void
        {
            if (auto structDesc = qualType.GetResolvedType()->GetStructDesc()) {
                if (structDesc->decl == referenceDecl) {
                    AddReferenceToken(qualType.GetTypeNameTok());
                }
            }
        }

        auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void
        {
            if (expr.GetResolvedDecl() == referenceDecl) {
                AddReferenceToken(expr.GetNameToken());
            }
        }
        auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
        {
            if (expr.GetResolvedDecl() == referenceDecl) {
                AddReferenceToken(expr.GetNameToken());
            }
        }
        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
        {
            if (expr.GetResolvedFunction() == referenceDecl) {
                AddReferenceToken(expr.GetNameToken());
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
        {
            if (includeDeclaration && &decl == referenceDecl.GetDecl()) {
                AddReferenceToken(decl.GetNameToken());
            }
        }
        auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
        {
            if (includeDeclaration && &decl == referenceDecl.GetDecl()) {
                AddReferenceToken(decl.GetDeclarators()[referenceDecl.GetIndex()].nameToken);
            }
        }
        auto VisitAstFieldDecl(const AstStructFieldDecl& decl) -> void
        {
            if (includeDeclaration && &decl == referenceDecl.GetDecl()) {
                AddReferenceToken(decl.GetDeclarators()[referenceDecl.GetIndex()].nameToken);
            }
        }
        auto VisitAstParamDecl(const AstParamDecl& decl) -> void
        {
            if (includeDeclaration && &decl == referenceDecl) {
                if (auto declarator = decl.GetDeclarator()) {
                    AddReferenceToken(declarator->nameToken);
                }
            }
        }
        auto VisitAstStructDecl(const AstStructDecl& decl) -> void
        {
            if (includeDeclaration && &decl == referenceDecl.GetDecl()) {
                if (decl.GetNameToken()) {
                    AddReferenceToken(*decl.GetNameToken());
                }
            }
        }
        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
        {
            if (includeDeclaration && &decl == referenceDecl.GetDecl()) {
                if (decl.GetDeclarator()) {
                    AddReferenceToken(decl.GetDeclarator()->nameToken);
                }
            }
        }

    private:
        auto AddReferenceToken(const AstSyntaxToken& token) -> void
        {
            // FIXME: Support reference from included files
            if (GetInfo().IsSpelledInMainFile(token.id)) {
                output.push_back(lsp::Location{documentUri, ToLspRange(GetInfo().LookupExpandedTextRange(token))});
            }
        }
    };

    auto GetReferenceOptions(const ReferenceConfig& config) -> lsp::ReferenceOptions
    {
        return lsp::ReferenceOptions{};
    }

    // FIXME: Support includeDeclaration
    // FIXME: Support type references
    // FIXME: Support swizzle
    // FIXME: Reference in MACROs?
    auto HandleReferences(const ReferenceConfig& config, const LanguageQueryInfo& info,
                          const lsp::ReferenceParams& params) -> std::vector<lsp::Location>
    {
        auto accessInfo = info.QuerySymbolByPosition(FromLspPosition(params.position));
        if (!accessInfo || !accessInfo->symbolDecl.IsValid()) {
            return {};
        }

        std::vector<lsp::Location> result;
        CollectReferenceVisitor{result, info, params.textDocument.uri, accessInfo->symbolDecl,
                                params.context.includeDeclaration}
            .Execute();
        return result;
    }
} // namespace glsld