
#include "Feature/Reference.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"

namespace glsld
{
    class CollectReferenceVisitor : public LanguageQueryVisitor<CollectReferenceVisitor, std::vector<lsp::Location>>
    {
    private:
        lsp::DocumentUri documentUri;

        const AstDecl* referenceDecl;

        bool includeDeclaration;

        std::vector<lsp::Location> result;

        auto AddReferenceToken(const AstSyntaxToken& token) -> void
        {
            // FIXME: Support reference from included files
            if (GetInfo().IsSpelledInMainFile(token.id)) {
                result.push_back(lsp::Location{documentUri, ToLspRange(GetInfo().LookupExpandedTextRange(token))});
            }
        }

    public:
        CollectReferenceVisitor(const LanguageQueryInfo& info, lsp::DocumentUri uri, const AstDecl* referenceDecl,
                                bool includeDeclaration)
            : LanguageQueryVisitor(info), documentUri(std::move(uri)), referenceDecl(referenceDecl),
              includeDeclaration(includeDeclaration)
        {
        }

        auto Finish() -> std::vector<lsp::Location> GLSLD_AST_VISITOR_OVERRIDE
        {
            return std::move(result);
        }

        auto VisitAstQualType(const AstQualType& qualType) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (auto structDesc = qualType.GetResolvedType()->GetStructDesc()) {
                if (structDesc->decl == referenceDecl) {
                    AddReferenceToken(qualType.GetTypeNameTok());
                }
            }
        }

        auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (expr.GetResolvedDecl() == referenceDecl) {
                AddReferenceToken(expr.GetNameToken());
            }
        }
        auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (expr.GetResolvedDecl() == referenceDecl) {
                AddReferenceToken(expr.GetNameToken());
            }
        }
        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (expr.GetResolvedFunction() == referenceDecl) {
                AddReferenceToken(expr.GetNameToken());
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                AddReferenceToken(decl.GetNameToken());
            }
        }
        auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                AddReferenceToken(decl.GetNameToken());
            }
        }
        auto VisitAstStructFieldDeclaratorDecl(const AstStructFieldDeclaratorDecl& decl)
            -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                AddReferenceToken(decl.GetNameToken());
            }
        }
        auto VisitAstParamDecl(const AstParamDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                if (auto declarator = decl.GetDeclarator()) {
                    AddReferenceToken(declarator->nameToken);
                }
            }
        }
        auto VisitAstStructDecl(const AstStructDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                if (decl.GetNameToken()) {
                    AddReferenceToken(*decl.GetNameToken());
                }
            }
        }
        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                if (decl.GetDeclarator()) {
                    AddReferenceToken(decl.GetDeclarator()->nameToken);
                }
            }
        }
        auto VisitAstBlockFieldDeclaratorDecl(const AstBlockFieldDeclaratorDecl& decl)
            -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (includeDeclaration && &decl == referenceDecl) {
                AddReferenceToken(decl.GetNameToken());
            }
        }
    };

    auto GetReferenceOptions(const ReferenceConfig& config) -> std::optional<lsp::ReferenceOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::ReferenceOptions{};
    }

    // FIXME: Support includeDeclaration
    // FIXME: Support type references
    // FIXME: Support swizzle
    // FIXME: Reference in MACROs?
    auto HandleReferences(const ReferenceConfig& config, const LanguageQueryInfo& info,
                          const lsp::ReferenceParams& params) -> std::vector<lsp::Location>
    {
        if (!config.enable) {
            return {};
        }

        auto symbolInfo = info.QuerySymbolByPosition(FromLspPosition(params.position));
        if (!symbolInfo) {
            return {};
        }

        std::vector<lsp::Location> result;
        if (symbolInfo->symbolType == SymbolDeclType::HeaderName) {
            // Do nothing for header name
        }
        else if (symbolInfo->symbolType == SymbolDeclType::Macro) {
            for (const auto& occurrence : info.GetPreprocessInfo().GetAllOccurrences()) {
                if (auto macroInfo = occurrence.GetMacroInfo(); macroInfo) {
                    if (macroInfo->definition == symbolInfo->ppSymbolOccurrence->GetMacroInfo()->definition) {
                        if (!params.context.includeDeclaration &&
                            macroInfo->occurrenceType == PPMacroOccurrenceType::Define) {
                            continue;
                        }

                        result.push_back(lsp::Location{
                            params.textDocument.uri,
                            ToLspRange(occurrence.GetSpelledRange()),
                        });
                    }
                }
            }
        }
        else if (symbolInfo->astSymbolOccurrence && symbolInfo->symbolDecl) {
            result = TraverseAst(CollectReferenceVisitor{info, params.textDocument.uri, symbolInfo->symbolDecl,
                                                         params.context.includeDeclaration},
                                 info.GetUserFileAst());
        }

        return result;
    }
} // namespace glsld