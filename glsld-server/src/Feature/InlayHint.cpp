#include "Feature/InlayHint.h"
#include "Ast/Expr.h"
#include "Ast/Type.h"
#include "Basic/Common.h"
#include "Basic/SourceInfo.h"
#include "Compiler/SyntaxToken.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"
#include "Support/StringView.h"
#include <ranges>

namespace glsld
{
    // FIXME: handle display range better
    class InlayHintCollector : public LanguageQueryVisitor<InlayHintCollector, std::vector<lsp::InlayHint>>
    {
    private:
        const InlayHintConfig& config;
        TextRange displayRange;

        std::vector<lsp::InlayHint> result;

    public:
        InlayHintCollector(const LanguageQueryInfo& info, const InlayHintConfig& config, TextRange range)
            : LanguageQueryVisitor(info), config(config), displayRange(range)
        {
        }

        auto Finish() -> std::vector<lsp::InlayHint> GLSLD_AST_VISITOR_OVERRIDE
        {
            return std::move(result);
        }

        auto VisitAstInitializerList(const AstInitializerList& ilist) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (!config.enableInitializerHint) {
                return;
            }

            if (auto structDesc = ilist.GetDeducedType()->GetStructDesc()) {
                for (const auto& [memberDesc, initializer] : std::views::zip(structDesc->members, ilist.GetItems())) {
                    TryAddInlayHintBefore(*initializer, fmt::format(".{}:", memberDesc.name));
                }
            }
            else if (auto arrayDesc = ilist.GetDeducedType()->GetArrayDesc()) {
                for (size_t i = 0; i < ilist.GetItems().size() && i < arrayDesc->dimSize; ++i) {
                    TryAddInlayHintBefore(*ilist.GetItems()[i], fmt::format("[{}]:", i));
                }
            }
            else if (auto vecDesc = ilist.GetDeducedType()->GetVectorDesc()) {
                for (auto [swizzleName, item] :
                     std::views::zip(StringView{"xyzw"}.Take(vecDesc->vectorSize), ilist.GetItems())) {
                    TryAddInlayHintBefore(*item, fmt::format(".{}:", swizzleName));
                }
            }
            else if (auto matDesc = ilist.GetDeducedType()->GetMatrixDesc()) {
                // FIXME: handle row-major matrix correctly
                for (size_t i = 0; i < ilist.GetItems().size() && i < matDesc->dimRow; ++i) {
                    TryAddInlayHintBefore(*ilist.GetItems()[i], fmt::format("[{}]:", i));
                }
            }
        }

        auto VisitAstImplicitCastExpr(const AstImplicitCastExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (!config.enableImplicitCastHint) {
                return;
            }
            if (expr.GetDeducedType()->IsError() || expr.GetSyntaxRange().GetTokenCount() > 1) {
                // FIXME: handle multi-token case. For example float(1+2)?
                return;
            }

            TryAddInlayHintBefore(expr, fmt::format("({})", expr.GetDeducedType()->GetDebugName()));
        }

        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (!config.enableArgumentNameHint) {
                return;
            }
            if (!expr.GetResolvedFunction()) {
                return;
            }

            const auto paramDeclList = expr.GetResolvedFunction()->GetParams();
            for (size_t i = 0; i < paramDeclList.size(); ++i) {
                GLSLD_ASSERT(i < expr.GetArgs().size());

                auto paramDecl = paramDeclList[i];
                auto argExpr   = expr.GetArgs()[i];

                StringView outputHint = "";
                if (paramDecl->IsOutputParam()) {
                    outputHint = "&";
                }
                StringView paramNameHint = "";
                if (paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->nameToken.IsIdentifier()) {
                    paramNameHint = paramDecl->GetDeclarator()->nameToken.text.StrView();
                }

                if (!outputHint.Empty() || !paramNameHint.Empty()) {
                    TryAddInlayHintBefore(*argExpr, fmt::format("{}{}:", outputHint, paramNameHint));
                }
            }
        }

        auto VisitAstConstructorCallExpr(const AstConstructorCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            // Handle hints for implicit array size
            // FIXME: handle multi-dimensional array
            if (config.enableImplicitArraySizeHint) {
                if (auto arraySpec = expr.GetConstructedType()->GetArraySpec();
                    arraySpec && arraySpec->GetSizeList().back() == nullptr) {
                    if (auto arrayDesc = expr.GetDeducedType()->GetArrayDesc(); arrayDesc) {
                        TryAddInlayHint(arraySpec->GetSyntaxRange().GetBeginID() + 1,
                                        fmt::format("{}", arrayDesc->dimSize), false, false);
                    }
                }
            }

            // Handle hints for constructor arguments
            if (!config.enableInitializerHint) {
                return;
            }
            const auto args = expr.GetArgs();
            if (auto structDesc = expr.GetDeducedType()->GetStructDesc()) {
                for (const auto& [memberDesc, argExpr] : std::views::zip(structDesc->members, args)) {
                    TryAddInlayHintBefore(*argExpr, fmt::format(".{}:", memberDesc.name));
                }
            }
            else if (auto arrayDesc = expr.GetDeducedType()->GetArrayDesc()) {
                for (size_t i = 0; i < args.size() && i < arrayDesc->dimSize; ++i) {
                    TryAddInlayHintBefore(*args[i], fmt::format("[{}]:", i));
                }
            }
            else if (auto vecDesc = expr.GetDeducedType()->GetVectorDesc()) {
                StringView remainingSwizzleNames = StringView{"xyzw"}.Take(vecDesc->vectorSize);
                if (args.size() == 1 && args.front()->GetDeducedType()->IsScalar()) {
                    TryAddInlayHintBefore(*args.front(), fmt::format(".{}:", remainingSwizzleNames));
                }
                else {
                    for (auto argExpr : args) {
                        if (remainingSwizzleNames.Empty()) {
                            break;
                        }

                        StringView swizzleName =
                            remainingSwizzleNames.Take(argExpr->GetDeducedType()->GetElementScalarCount().value_or(1));
                        remainingSwizzleNames = remainingSwizzleNames.Drop(swizzleName.Size());
                        TryAddInlayHintBefore(*argExpr, fmt::format(".{}:", swizzleName));
                    }
                }
            }
            else if (auto matDesc = expr.GetDeducedType()->GetMatrixDesc()) {
                if (args.size() == 1 && args.front()->GetDeducedType()->IsScalar()) {
                    TryAddInlayHintBefore(*args.front(), "diag:");
                }
                else if (args.size() == 1 && args.front()->GetDeducedType()->IsMatrix()) {
                    TryAddInlayHintBefore(*args.front(), "m:");
                }
                else {
                    // FIXME: handle composing a matrix from scalar/vector
                }
            }
        }

        auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (!config.enableImplicitArraySizeHint) {
                return;
            }

            // Handle hints for implicit array size
            // FIXME: handle multi-dimensional array
            if (auto arraySpec = decl.GetArraySpec();
                arraySpec && arraySpec->GetSizeList().back() == nullptr && decl.GetInitializer()) {
                if (auto arrayDesc = decl.GetResolvedType()->GetArrayDesc(); arrayDesc) {
                    TryAddInlayHint(arraySpec->GetSyntaxRange().GetBeginID() + 1, fmt::format("{}", arrayDesc->dimSize),
                                    false, false);
                }
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (!config.enableBlockEndHint) {
                return;
            }
            if (!decl.GetNameToken().IsIdentifier()) {
                return;
            }

            auto declTextRange = GetInfo().LookupExpandedTextRange(decl);
            if (declTextRange.GetNumLines() < config.blockEndHintLineThreshold) {
                return;
            }

            // FIXME: don't show hints if any additional tokens are present in the line
            TryAddInlayHint(declTextRange.end, fmt::format("// {}", decl.GetNameToken().text.StrView()), true, false);
        }

    private:
        auto TryAddInlayHint(TextPosition position, std::string label, bool paddingLeft, bool paddingRight) -> void
        {
            if (displayRange.Contains(position)) {
                result.push_back(lsp::InlayHint{
                    .position     = ToLspPosition(position),
                    .label        = std::move(label),
                    .paddingLeft  = paddingLeft,
                    .paddingRight = paddingRight,
                });
            }
        }

        auto TryAddInlayHint(SyntaxTokenID token, std::string label, bool paddingLeft, bool paddingRight) -> void
        {
            const auto range = GetInfo().LookupExpandedTextRange(token);
            if (range.IsEmpty()) {
                return;
            }

            TryAddInlayHint(range.start, std::move(label), paddingLeft, paddingRight);
        }

        auto TryAddInlayHintBefore(const AstNode& node, std::string label) -> void
        {
            const auto range = GetInfo().LookupExpandedTextRange(node);
            if (range.IsEmpty()) {
                return;
            }

            TryAddInlayHint(range.start, std::move(label), false, true);
        }
    };

    auto GetInlayHintsOptions(const InlayHintConfig& config) -> std::optional<lsp::InlayHintOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::InlayHintOptions{
            .resolveProvider = false,
        };
    }

    auto HandleInlayHints(const InlayHintConfig& config, const LanguageQueryInfo& info,
                          const lsp::InlayHintParams& params) -> std::vector<lsp::InlayHint>
    {
        if (!config.enable) {
            return {};
        }

        return TraverseAst(InlayHintCollector{info, config, FromLspRange(params.range)}, info.GetUserFileAst());
    }

} // namespace glsld