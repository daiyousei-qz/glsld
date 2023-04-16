
#include "Compiler.h"
#include "Protocol.h"
#include "ModuleVisitor.h"
#include "AstHelper.h"

#include <vector>

namespace glsld
{
    class CollectReferenceVisitor : public ModuleVisitor<CollectReferenceVisitor>
    {
    public:
        CollectReferenceVisitor(std::vector<lsp::Location>& output, const CompilerObject& compiler,
                                lsp::DocumentUri uri, DeclView referenceDecl)
            : ModuleVisitor(compiler), documentUri(std::move(uri)), output(output), referenceDecl(referenceDecl)
        {
        }

        auto Execute() -> void
        {
            this->Traverse();
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            if (expr.GetAccessedDecl() == referenceDecl) {
                output.push_back(lsp::Location{
                    documentUri, ToLspRange(this->GetLexContext().LookupExpandedTextRange(expr.GetSyntaxRange()))});
            }
        }

    private:
        std::vector<lsp::Location>& output;

        lsp::DocumentUri documentUri;
        DeclView referenceDecl;
    };

    // FIXME: Support includeDeclaration
    // FIXME: Support type references
    // FIXME: Support swizzle
    auto ComputeReferences(const CompilerObject& compilerObject, const lsp::DocumentUri& uri, lsp::Position position,
                           bool includeDeclaration) -> std::vector<lsp::Location>
    {
        auto declTokInfo = FindDeclToken(compilerObject, FromLspPosition(position));
        if (!declTokInfo || !declTokInfo->accessedDecl.IsValid()) {
            return {};
        }

        std::vector<lsp::Location> result;
        CollectReferenceVisitor{result, compilerObject, uri, declTokInfo->accessedDecl}.Execute();
        return result;
    }
} // namespace glsld