#include "Server/SourceReconstruction.h"

#include "Ast/Eval.h"

namespace glsld
{
    auto SourceReconstructionBuilder::AppendInitializer(const AstInitializer& initializer) -> void
    {
        // FIXME: reconstruct the value expression properly
        Append(" = /* ... */");
    }
    auto SourceReconstructionBuilder::AppendArraySpec(const AstArraySpec& arraySpec) -> void
    {
        for (auto dimSizeExpr : arraySpec.GetSizeList()) {
            if (dimSizeExpr) {
                Append("[{}]", EvalAstInitializer(*dimSizeExpr));
            }
            else {
                Append("[]");
            }
        }
    }
    auto SourceReconstructionBuilder::AppendTypeQualifierSeq(const AstTypeQualifierSeq& typeQualifierSeq) -> void
    {
        if (!typeQualifierSeq.GetLayoutQuals().empty()) {
            Append("layout(");

            for (const auto& [i, layoutItem] : std::views::enumerate(typeQualifierSeq.GetLayoutQuals())) {
                if (i > 0) {
                    Append(", ");
                }

                Append("{}", layoutItem.idToken.text.StrView());
                if (layoutItem.value) {
                    AppendInitializer(*layoutItem.value);
                }
            }

            Append(")");
        }

        auto qualStr = typeQualifierSeq.GetQualGroup().ToString();
        if (!qualStr.empty()) {
            if (!typeQualifierSeq.GetLayoutQuals().empty()) {
                Append(" ");
            }
            Append("{}", qualStr);
        }
    }
    auto SourceReconstructionBuilder::AppendQualType(const AstQualType& qualType) -> void
    {
        if (qualType.GetQualifiers()) {
            AppendTypeQualifierSeq(*qualType.GetQualifiers());
            Append(" ");
        }

        if (qualType.GetStructDecl()) {
            AppendStructDecl(*qualType.GetStructDecl());
        }
        else {
            Append("{}", qualType.GetTypeNameTok().text.StrView());
        }

        if (qualType.GetArraySpec()) {
            AppendArraySpec(*qualType.GetArraySpec());
        }
    }
    auto SourceReconstructionBuilder::AppendDeclarator(const AstSyntaxToken& nameToken, const AstArraySpec* arraySpec,
                                                       const AstInitializer* initializer) -> void
    {
        Append("{}", nameToken.text.StrView());
        if (arraySpec) {
            AppendArraySpec(*arraySpec);
        }
        if (initializer) {
            AppendInitializer(*initializer);
        }
    }
    auto SourceReconstructionBuilder::AppendQualTypeDeclarator(const AstQualType& qualType,
                                                               const AstSyntaxToken& nameToken,
                                                               const AstArraySpec* arraySpec,
                                                               const AstInitializer* initializer) -> void
    {
        AppendQualType(qualType);
        Append(" ");
        AppendDeclarator(nameToken, arraySpec, initializer);
    }
    auto SourceReconstructionBuilder::AppendParamDecl(const AstParamDecl& paramDecl) -> void
    {
        if (paramDecl.GetQualType()) {
            AppendQualType(*paramDecl.GetQualType());
            Append(" ");
        }

        if (paramDecl.GetDeclarator()) {
            AppendDeclarator(paramDecl.GetDeclarator()->nameToken, paramDecl.GetDeclarator()->arraySpec, nullptr);
        }
    }
    auto SourceReconstructionBuilder::AppendFunctionDecl(const AstFunctionDecl& decl) -> void
    {
        AppendQualType(*decl.GetReturnType());
        Append(" {}(", decl.GetNameToken().text.StrView());

        for (const auto& [i, paramDecl] : std::views::enumerate(decl.GetParams())) {
            if (i > 0) {
                Append(", ");
            }

            AppendParamDecl(*paramDecl);
        }

        Append(")");
    }
    auto SourceReconstructionBuilder::AppendStructDecl(const AstStructDecl& decl) -> void
    {
        // FIXME: reconstruct from Type
        Append("struct");
        if (decl.GetNameToken()) {
            Append(" {}", decl.GetNameToken()->text.StrView());
        }
        Append(" {{\n");
        for (const auto& memberDecl : decl.GetMembers()) {
            for (const auto& declaratorDecl : memberDecl->GetDeclarators()) {
                Append("    ");
                AppendQualTypeDeclarator(*memberDecl->GetQualType(), declaratorDecl->GetNameToken(),
                                         declaratorDecl->GetArraySpec(), nullptr);
                Append(";\n");
            }
        }
        Append("}}");
    }
    auto SourceReconstructionBuilder::AppendInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
    {
        Append("{}", decl.GetQuals()->GetQualGroup().ToString());
        if (decl.GetNameToken().IsValid()) {
            Append(" {}", decl.GetNameToken().text.StrView());
        }
        Append(" {{\n");
        for (const auto& memberDecl : decl.GetMembers()) {
            for (const auto& declaratorDecl : memberDecl->GetDeclarators()) {
                Append("    ");
                AppendQualTypeDeclarator(*memberDecl->GetQualType(), declaratorDecl->GetNameToken(),
                                         declaratorDecl->GetArraySpec(), nullptr);
                Append(";\n");
            }
        }
        Append("}}");
        if (const auto& declarator = decl.GetDeclarator()) {
            Append(" ");
            AppendDeclarator(declarator->nameToken, declarator->arraySpec, nullptr);
        }
    }
    auto SourceReconstructionBuilder::Print(const AstDecl& decl) -> std::string
    {
        buffer.clear();

        if (auto paramDecl = decl.As<AstParamDecl>()) {
            AppendParamDecl(*paramDecl);
        }
        else if (auto funcDecl = decl.As<AstFunctionDecl>()) {
            AppendFunctionDecl(*funcDecl);
            Append(";");
        }
        else if (auto varDeclaratorDecl = decl.As<AstVariableDeclaratorDecl>()) {
            AppendQualTypeDeclarator(*varDeclaratorDecl->GetQualType(), varDeclaratorDecl->GetNameToken(),
                                     varDeclaratorDecl->GetArraySpec(), varDeclaratorDecl->GetInitializer());
            Append(";");
        }
        else if (auto structMemberDeclaratorDecl = decl.As<AstStructFieldDeclaratorDecl>()) {
            AppendQualTypeDeclarator(
                *structMemberDeclaratorDecl->GetQualType(), structMemberDeclaratorDecl->GetNameToken(),
                structMemberDeclaratorDecl->GetArraySpec(), structMemberDeclaratorDecl->GetInitializer());
            Append(";");
        }
        else if (auto blockMemberDeclaratorDecl = decl.As<AstBlockFieldDeclaratorDecl>()) {
            AppendQualTypeDeclarator(
                *blockMemberDeclaratorDecl->GetQualType(), blockMemberDeclaratorDecl->GetNameToken(),
                blockMemberDeclaratorDecl->GetArraySpec(), blockMemberDeclaratorDecl->GetInitializer());
            Append(";");
        }
        else if (auto structDecl = decl.As<AstStructDecl>()) {
            AppendStructDecl(*structDecl);
            Append(";");
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
            AppendInterfaceBlockDecl(*blockDecl);
            Append(";");
        }

        return fmt::to_string(buffer);
    }
} // namespace glsld