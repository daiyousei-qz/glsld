#pragma once
#include "Ast/Eval.h"

namespace glsld
{
    class SourceReconstructionBuilder
    {
    private:
        fmt::memory_buffer buffer;

        auto AppendInitializer(const AstInitializer& initializer) -> void
        {
            // FIXME: reconstruct the value expression properly
            fmt::format_to(std::back_inserter(buffer), " = ...");
        }

        auto AppendArraySpec(const AstArraySpec& arraySpec) -> void
        {
            for (auto dimSizeExpr : arraySpec.GetSizeList()) {
                if (dimSizeExpr) {
                    auto dimSizeExprValue = EvalAstExpr(*dimSizeExpr);
                    fmt::format_to(std::back_inserter(buffer), "[{}]", dimSizeExprValue);
                }
                else {
                    fmt::format_to(std::back_inserter(buffer), "[]");
                }
            }
        }

        auto AppendTypeQualifierSeq(const AstTypeQualifierSeq& typeQualifierSeq) -> void
        {
            if (!typeQualifierSeq.GetLayoutQuals().empty()) {
                fmt::format_to(std::back_inserter(buffer), "layout(");

                for (const auto& [i, layoutItem] : std::views::enumerate(typeQualifierSeq.GetLayoutQuals())) {
                    if (i > 0) {
                        fmt::format_to(std::back_inserter(buffer), ", ");
                    }

                    fmt::format_to(std::back_inserter(buffer), "{}", layoutItem.idToken.text.StrView());
                    if (layoutItem.value) {
                        AppendInitializer(*layoutItem.value);
                    }
                }

                fmt::format_to(std::back_inserter(buffer), ")");
            }

            auto qualStr = typeQualifierSeq.GetQualGroup().ToString();
            if (!qualStr.empty()) {
                if (!typeQualifierSeq.GetLayoutQuals().empty()) {
                    fmt::format_to(std::back_inserter(buffer), " ");
                }
                fmt::format_to(std::back_inserter(buffer), "{}", qualStr);
            }
        }

        auto AppendQualType(const AstQualType& qualType) -> void
        {
            if (qualType.GetQualifiers()) {
                AppendTypeQualifierSeq(*qualType.GetQualifiers());
                fmt::format_to(std::back_inserter(buffer), " ");
            }

            if (qualType.GetStructDecl()) {
                AppendStructDecl(*qualType.GetStructDecl());
            }
            else {
                fmt::format_to(std::back_inserter(buffer), "{}", qualType.GetTypeNameTok().text.StrView());
            }

            if (qualType.GetArraySpec()) {
                AppendArraySpec(*qualType.GetArraySpec());
            }
        }

        auto AppendDeclarator(const AstSyntaxToken& nameToken, const AstArraySpec* arraySpec,
                              const AstInitializer* initializer) -> void
        {
            fmt::format_to(std::back_inserter(buffer), "{}", nameToken.text.StrView());
            if (arraySpec) {
                AppendArraySpec(*arraySpec);
            }
            if (initializer) {
                AppendInitializer(*initializer);
            }
        }

        auto AppendQualTypeDeclarator(const AstQualType& qualType, const AstSyntaxToken& nameToken,
                                      const AstArraySpec* arraySpec, const AstInitializer* initializer) -> void
        {
            AppendQualType(qualType);
            fmt::format_to(std::back_inserter(buffer), " ");
            AppendDeclarator(nameToken, arraySpec, initializer);
        }

        auto AppendFunctionDecl(const AstFunctionDecl& decl) -> void
        {
            AppendQualType(*decl.GetReturnType());
            fmt::format_to(std::back_inserter(buffer), " {}", decl.GetNameToken().text.StrView());
        }

        auto AppendStructDecl(const AstStructDecl& decl) -> void
        {
            // FIXME: reconstruct from Type
            fmt::format_to(std::back_inserter(buffer), "struct");
            if (decl.GetNameToken()) {
                fmt::format_to(std::back_inserter(buffer), " {}", decl.GetNameToken()->text.StrView());
            }
            fmt::format_to(std::back_inserter(buffer), " {{ ... }}");
        }

        auto AppendInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
        {
            fmt::format_to(std::back_inserter(buffer), "{} {} {{ ... }}", decl.GetQuals()->GetQualGroup().ToString(),
                           decl.GetNameToken().text.StrView());
            if (const auto& declarator = decl.GetDeclarator()) {
                fmt::format_to(std::back_inserter(buffer), " ");
                AppendDeclarator(declarator->nameToken, declarator->arraySpec, nullptr);
            }
        }

    public:
        SourceReconstructionBuilder() = default;

        auto Print(const AstDecl& decl) -> std::string
        {
            buffer.clear();

            if (auto funcDecl = decl.As<AstFunctionDecl>()) {
                AppendFunctionDecl(*funcDecl);
            }
            else if (auto varDeclaratorDecl = decl.As<AstVariableDeclaratorDecl>()) {
                AppendQualTypeDeclarator(*varDeclaratorDecl->GetQualType(), varDeclaratorDecl->GetNameToken(),
                                         varDeclaratorDecl->GetArraySpec(), varDeclaratorDecl->GetInitializer());
            }
            else if (auto structDecl = decl.As<AstStructDecl>()) {
                AppendStructDecl(*structDecl);
            }
            else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
                AppendInterfaceBlockDecl(*blockDecl);
            }

            return fmt::to_string(buffer);
        }
    };
} // namespace glsld