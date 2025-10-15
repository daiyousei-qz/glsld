#pragma once
#include "Ast/Eval.h"
#include "Ast/Expr.h"
#include "Compiler/SyntaxToken.h"
#include "Server/Protocol.h"

#include <optional>

namespace glsld
{
    inline auto FromLspPosition(lsp::Position position) -> TextPosition
    {
        return TextPosition{
            .line      = static_cast<int>(position.line),
            .character = static_cast<int>(position.character),
        };
    }
    inline auto ToLspPosition(TextPosition position) -> lsp::Position
    {
        return lsp::Position{
            .line      = static_cast<uint32_t>(position.line),
            .character = static_cast<uint32_t>(position.character),
        };
    }

    inline auto FromLspRange(lsp::Range range) -> TextRange
    {
        return TextRange{
            FromLspPosition(range.start),
            FromLspPosition(range.end),
        };
    }
    inline auto ToLspRange(TextRange range) -> lsp::Range
    {
        return lsp::Range{
            .start = ToLspPosition(range.start),
            .end   = ToLspPosition(range.end),
        };
    }

    inline auto ApplySourceChange(std::string& sourceBuffer, TextRange range, StringView changedText) -> void
    {
        TextPosition cur = {};
        size_t index     = 0;

        // Find the begining index of the range
        for (; index < sourceBuffer.size(); ++index) {
            if (cur >= range.start) {
                break;
            }

            if (sourceBuffer[index] == '\n') {
                cur.line += 1;
                cur.character = 0;
            }
            else {
                cur.character += 1;
            }
        }

        size_t indexBegin = index;

        // Find the ending index of the range
        for (; index < sourceBuffer.size(); ++index) {
            if (cur >= range.end) {
                break;
            }

            if (sourceBuffer[index] == '\n') {
                cur.line += 1;
                cur.character = 0;
            }
            else {
                cur.character += 1;
            }
        }

        size_t indexEnd = index;

        sourceBuffer.replace(indexBegin, indexEnd - indexBegin, changedText.StdStrView());
    }

    // FIXME: where to put these functions?
    inline auto ReconstructSourceText(std::string& buffer, const Declarator& declarator) -> void
    {
        if (declarator.nameToken.IsIdentifier()) {
            buffer += declarator.nameToken.text.StrView();
            if (declarator.arraySpec) {
                for (auto dimSizeExpr : declarator.arraySpec->GetSizeList()) {
                    if (dimSizeExpr) {
                        auto dimSizeExprValue = EvalAstExpr(*dimSizeExpr);
                        if (dimSizeExprValue.IsScalarInt32()) {
                            buffer += fmt::format("[{}]", dimSizeExprValue.GetInt32Value());
                        }
                        else {
                            buffer += "[__error]";
                        }
                    }
                    else {
                        buffer += "[]";
                    }
                }
            }
        }
        else {
            buffer += "<error>";
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, const AstArraySpec& arraySpec) -> void
    {
        for (auto dimSizeExpr : arraySpec.GetSizeList()) {
            if (dimSizeExpr) {
                auto dimSizeExprValue = EvalAstExpr(*dimSizeExpr);
                if (dimSizeExprValue.IsScalarInt32()) {
                    buffer += fmt::format("[{}]", dimSizeExprValue.GetInt32Value());
                }
                else {
                    buffer += "[__error]";
                }
            }
            else {
                buffer += "[]";
            }
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, const AstQualType& type) -> void
    {
        // Reconstruct qualfiers
        if (type.GetQualifiers()) {
            const auto quals = type.GetQualifiers()->GetQualGroup();
            // Precision Qualifier
            if (quals.qHighp) {
                buffer += "highp ";
            }
            if (quals.qMediump) {
                buffer += "mediump ";
            }
            if (quals.qLowp) {
                buffer += "lowp ";
            }

            // Storage/Parameter qualifiers
            if (quals.qConst) {
                buffer += "const ";
            }
            if (quals.qIn) {
                buffer += "in ";
            }
            if (quals.qOut) {
                buffer += "out ";
            }
            if (quals.qInout) {
                buffer += "inout ";
            }
            if (quals.qAttribute) {
                buffer += "attribute ";
            }
            if (quals.qUniform) {
                buffer += "uniform ";
            }
            if (quals.qVarying) {
                buffer += "varying ";
            }
            if (quals.qBuffer) {
                buffer += "buffer ";
            }
            if (quals.qShared) {
                buffer += "shared ";
            }

            // Auxiliary storage qualifiers
            if (quals.qCentroid) {
                buffer += "centroid ";
            }
            if (quals.qSample) {
                buffer += "sample ";
            }
            if (quals.qPatch) {
                buffer += "patch ";
            }

            // Interpolation qualifiers
            if (quals.qSmooth) {
                buffer += "smooth ";
            }
            if (quals.qFlat) {
                buffer += "flat ";
            }
            if (quals.qNoperspective) {
                buffer += "noperspective ";
            }

            // Variance qualifier
            if (quals.qInvariant) {
                buffer += "invariant ";
            }

            // Precise qualifier
            if (quals.qPrecise) {
                buffer += "precise ";
            }

            // Memory qualifiers
            if (quals.qCoherent) {
                buffer += "coherent ";
            }
            if (quals.qVolatile) {
                buffer += "volatile ";
            }
            if (quals.qRestrict) {
                buffer += "restrict ";
            }
            if (quals.qReadonly) {
                buffer += "readonly ";
            }
            if (quals.qWriteonly) {
                buffer += "writeonly ";
            }

            // Extension: ray tracing
            if (quals.qRayPayloadEXT) {
                buffer += "rayPayloadEXT ";
            }
            if (quals.qRayPayloadInEXT) {
                buffer += "rayPayloadInEXT ";
            }
            if (quals.qHitAttributeEXT) {
                buffer += "hitAttributeEXT ";
            }
            if (quals.qCallableDataEXT) {
                buffer += "callableDataEXT ";
            }
            if (quals.qCallableDataInEXT) {
                buffer += "callableDataInEXT ";
            }
        }

        // FIXME: reconstruct from Type
        if (auto structDecl = type.GetStructDecl()) {
            buffer += "struct ";
            if (structDecl->GetNameToken()) {
                buffer += structDecl->GetNameToken()->text.StrView();
            }
            buffer += " { ... }";
        }
        else {
            buffer += type.GetTypeNameTok().text.StrView();
        }

        if (type.GetArraySpec()) {
            ReconstructSourceText(buffer, *type.GetArraySpec());
        }
    }

    inline auto ReconstructSourceText(std::string& buffer, const AstQualType& qualType, const AstSyntaxToken& nameToken,
                                      const AstArraySpec* arraySpec, const AstInitializer* initializer) -> void
    {
        ReconstructSourceText(buffer, qualType);
        buffer += " ";

        buffer += nameToken.text.StrView();
        if (arraySpec) {
            ReconstructSourceText(buffer, *arraySpec);
        }
        if (initializer) {
            buffer += " = ...";
        }
    }

    inline auto ReconstructSourceText(std::string& buffer, const AstParamDecl& decl) -> void
    {
        ReconstructSourceText(buffer, *decl.GetQualType());
        if (decl.GetDeclarator()) {
            buffer += " ";
            ReconstructSourceText(buffer, *decl.GetDeclarator());
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, const AstFunctionDecl& decl) -> void
    {
        ReconstructSourceText(buffer, *decl.GetReturnType());
        buffer += " ";

        buffer += decl.GetNameToken().text.StrView();

        buffer += "(";
        for (auto paramDecl : decl.GetParams()) {
            ReconstructSourceText(buffer, *paramDecl);
            buffer += ", ";
        }
        if (buffer.back() == ' ') {
            buffer.pop_back();
            buffer.pop_back();
        }
        buffer += ")";
    }
    inline auto ReconstructSourceText(std::string& buffer, const AstStructDecl& decl) -> void
    {
        buffer += "struct";
        if (decl.GetNameToken()) {
            buffer += " ";
            buffer += decl.GetNameToken()->text.StrView();
        }

        buffer += " {\n";

        for (auto memberDecl : decl.GetMembers()) {
            for (auto declaratorDecl : memberDecl->GetDeclarators()) {
                buffer += "    ";
                ReconstructSourceText(buffer, *memberDecl->GetQualType(), declaratorDecl->GetNameToken(),
                                      declaratorDecl->GetArraySpec(), declaratorDecl->GetInitializer());
                buffer += ";\n";
            }
        }

        buffer += "}\n";
    }
    inline auto ReconstructSourceText(std::string& buffer, const AstInterfaceBlockDecl& decl) -> void
    {
        QualifierGroup qual = decl.GetQuals()->GetQualGroup();
        if (qual.qUniform) {
            buffer += "uniform ";
        }
        else if (qual.qBuffer) {
            buffer += "buffer ";
        }
        else if (qual.qIn) {
            buffer += "in ";
        }
        else if (qual.qOut) {
            buffer += "out ";
        }
        else if (qual.qRayPayloadEXT) {
            buffer += "rayPayloadEXT ";
        }
        else if (qual.qRayPayloadInEXT) {
            buffer += "rayPayloadInEXT ";
        }
        else if (qual.qHitAttributeEXT) {
            buffer += "hitAttributeEXT ";
        }
        else if (qual.qCallableDataEXT) {
            buffer += "callableDataEXT ";
        }
        else if (qual.qCallableDataInEXT) {
            buffer += "callableDataInEXT ";
        }

        buffer += decl.GetNameToken().text.StrView();
        buffer += " {\n";

        for (auto memberDecl : decl.GetMembers()) {
            for (auto declaratorDecl : memberDecl->GetDeclarators()) {
                buffer += "    ";
                ReconstructSourceText(buffer, *memberDecl->GetQualType(), declaratorDecl->GetNameToken(),
                                      declaratorDecl->GetArraySpec(), declaratorDecl->GetInitializer());
                buffer += ";\n";
            }
        }

        buffer += "}";

        if (const auto& declarator = decl.GetDeclarator()) {
            buffer += " ";
            ReconstructSourceText(buffer, *declarator);
        }
        buffer += "\n";

        // FIXME: add members
    }

} // namespace glsld