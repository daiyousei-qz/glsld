#pragma once
#include "Ast.h"
#include "Common.h"
#include "Protocol.h"
#include "SyntaxToken.h"
#include <iterator>
#include <vector>
#include <string_view>
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
    inline auto ReconstructSourceText(std::string& buffer, const VariableDeclarator& declarator) -> void
    {
        buffer += declarator.declTok.text.StrView();
        if (declarator.arraySize) {
            for (auto dimSizeExpr : declarator.arraySize->GetSizeList()) {
                if (dimSizeExpr && dimSizeExpr->GetConstValue().HasIntValue()) {
                    buffer += fmt::format("[{}]", dimSizeExpr->GetConstValue().GetIntValue());
                }
                else {
                    buffer += "[]";
                }
            }
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, const AstArraySpec& arraySpec) -> void
    {
        for (auto dimSizeExpr : arraySpec.GetSizeList()) {
            if (dimSizeExpr && dimSizeExpr->GetConstValue().HasIntValue()) {
                buffer += fmt::format("[{}]", dimSizeExpr->GetConstValue().GetIntValue());
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
            const auto quals = type.GetQualifiers()->GetQualfierGroup();
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
        }

        // FIXME: reconstruct from TypeDesc
        if (auto structDecl = type.GetStructDecl()) {
            buffer += "struct ";
            if (structDecl->GetDeclToken()) {
                buffer += structDecl->GetDeclToken()->text.StrView();
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
    inline auto ReconstructSourceText(std::string& buffer, AstStructMemberDecl& decl, size_t index) -> void
    {
        // FIXME: this is exactly the same implementation with AstVariableDecl
        GLSLD_ASSERT(index < decl.GetDeclarators().size());

        ReconstructSourceText(buffer, *decl.GetType());
        buffer += " ";

        ReconstructSourceText(buffer, decl.GetDeclarators()[index]);
    }
    inline auto ReconstructSourceText(std::string& buffer, AstVariableDecl& decl, size_t index) -> void
    {
        GLSLD_ASSERT(index < decl.GetDeclarators().size());

        ReconstructSourceText(buffer, *decl.GetType());
        buffer += " ";

        ReconstructSourceText(buffer, decl.GetDeclarators()[index]);
    }
    inline auto ReconstructSourceText(std::string& buffer, AstParamDecl& decl) -> void
    {
        ReconstructSourceText(buffer, *decl.GetType());
        if (decl.GetDeclarator()) {
            buffer += " ";
            ReconstructSourceText(buffer, *decl.GetDeclarator());
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, AstFunctionDecl& decl) -> void
    {
        ReconstructSourceText(buffer, *decl.GetReturnType());
        buffer += " ";

        buffer += decl.GetName().text.StrView();

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
    inline auto ReconstructSourceText(std::string& buffer, AstStructDecl& decl) -> void
    {
        buffer += "struct";
        if (decl.GetDeclToken()) {
            buffer += " ";
            buffer += decl.GetDeclToken()->text.StrView();
        }
    }

} // namespace glsld