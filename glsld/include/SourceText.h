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

    struct SourcePiece
    {
        std::optional<TextRange> range;
        std::string_view text;
    };

    // inline auto Contains(TextRange range, TextPosition position) -> bool
    // {
    //     if (position.line < range.start.line ||
    //         (position.line == range.start.line && position.character < range.start.character)) {
    //         return false;
    //     }
    //     if (position.line > range.end.line ||
    //         (position.line == range.end.line && position.character >= range.end.character)) {
    //         return false;
    //     }

    //     return true;
    // }

    // inline auto ComputeEndPosition(std::string_view text, TextPosition pieceStart)
    // {
    //     TextPosition cur = pieceStart;
    //     for (auto it = text.begin(); it != text.end(); ++it) {
    //         auto ch = *it;
    //         if (ch == '\n') {
    //             cur.line += 1;
    //             cur.character = 0;
    //         }
    //         else {
    //             cur.character += 1;
    //         }
    //     }

    //     return cur;
    // }

    // inline auto SplitSourcePiece(std::string_view text, TextPosition startPosition, TextPosition midPosition,
    //                              std::optional<TextPosition> endPosition)
    //     -> std::tuple<TextPosition, std::string_view, std::string_view>
    // {
    //     if (endPosition) {
    //         if (!Contains(TextRange{.start = startPosition, .end = *endPosition}, midPosition)) {
    //             return {*endPosition, text, std::string_view{}};
    //         }
    //     }

    //     TextPosition cur = startPosition;
    //     for (auto it = text.begin(); it != text.end(); ++it) {
    //         auto ch = *it;
    //         if (cur.line >= midPosition.line && cur.character >= midPosition.character) {
    //             return {cur, std::string_view{text.begin(), it}, std::string_view{it, text.end()}};
    //         }

    //         if (ch == '\n') {
    //             cur.line += 1;
    //             cur.character = 0;
    //         }
    //         else {
    //             cur.character += 1;
    //         }
    //     }

    //     return {cur, text, std::string_view{}};
    // }

    // inline auto ApplyTextChange(std::span<SourcePiece const> currentSources, TextRange changeRange,
    //                             std::string_view text) -> std::vector<SourcePiece>
    // {
    //     std::vector<SourcePiece> result;
    //     bool enteredChangeRange = false;
    //     bool exitedChangeRange  = false;
    //     for (const auto& piece : currentSources) {
    //         if (exitedChangeRange) {
    //             result.push_back(SourcePiece{
    //                 .range = std::nullopt,
    //                 .text  = piece.text,
    //             });
    //         }
    //         else {
    //             std::string_view currentPieceText = piece.text;

    //             TextPosition currentPieceStart = {};
    //             if (!result.empty()) {
    //                 GLSLD_ASSERT(result.back().range.has_value());
    //                 currentPieceStart = result.back().range->end;
    //             }

    //             std::optional<TextPosition> currentPieceEnd = std::nullopt;
    //             if (piece.range) {
    //                 currentPieceEnd = piece.range->end;
    //             }

    //             if (!enteredChangeRange) {
    //                 auto [posEnd, lhs, rhs] =
    //                     SplitSourcePiece(currentPieceText, currentPieceStart, changeRange.start, currentPieceEnd);

    //                 if (!lhs.empty()) {
    //                     // lhs holds everything before the start
    //                     result.push_back(SourcePiece{
    //                         .range = TextRange{.start = currentPieceStart, .end = posEnd},
    //                         .text  = lhs,
    //                     });
    //                 }

    //                 if (!rhs.empty()) {
    //                     // we've find the start
    //                     enteredChangeRange = true;

    //                     // update current piece info so we could continue finding end position
    //                     currentPieceText  = rhs;
    //                     currentPieceStart = posEnd;
    //                 }
    //             }

    //             if (enteredChangeRange) {
    //                 auto [posEnd, lhs, rhs] =
    //                     SplitSourcePiece(currentPieceText, currentPieceStart, changeRange.end, currentPieceEnd);

    //                 if (!rhs.empty()) {
    //                     // we've find the end
    //                     result.push_back(SourcePiece{
    //                         .range = std::nullopt,
    //                         .text  = text,
    //                     });

    //                     result.push_back(SourcePiece{
    //                         .range = std::nullopt,
    //                         .text  = rhs,
    //                     });

    //                     exitedChangeRange = true;
    //                 }
    //             }
    //         }
    //     }

    //     if (!enteredChangeRange) {
    //         result.push_back(SourcePiece{
    //             .range = std::nullopt,
    //             .text  = text,
    //         });
    //     }

    //     return result;
    // }

    // inline auto ToString(std::span<SourcePiece const> currentSources) -> std::string
    // {
    //     std::string result;
    //     for (const auto& piece : currentSources) {
    //         result += piece.text;
    //     }

    //     return result;
    // }

    inline auto ApplyContentChange(const std::string& input, const lsp::TextDocumentContentChangeEvent& changeEvent)
        -> std::string
    {
        std::string result;
        result.reserve(input.size() + changeEvent.text.size());

        int line          = 0;
        int charcter      = 0;
        auto changedRange = changeEvent.range;
        for (auto it = input.begin(); it != input.end(); ++it) {
            auto ch = *it;
            if (line >= changedRange->start.line && charcter >= changedRange->start.character) {
                if (line >= changedRange->end.line && charcter >= changedRange->end.character) {
                    std::ranges::copy(changeEvent.text, std::back_inserter(result));
                    std::ranges::copy(it, input.end(), std::back_inserter(result));
                    break;
                }
                continue;
            }

            result.push_back(ch);
            if (ch == '\n') {
                line += 1;
                charcter = 0;
            }
            else {
                charcter += 1;
            }
        }

        return result;
    }

    // FIXME: where to put these functions?
    inline auto ReconstructSourceText(const SyntaxToken& tok) -> std::string_view
    {
        if (tok.klass != TokenKlass::Error) {
            return tok.text.StrView();
        }
        else {
            return "<error>";
        }
    }

    inline auto ReconstructSourceText(AstQualType* type) -> std::string
    {
        std::string result;

        // Reconstruct qualfiers
        if (type->GetQualifiers()) {
            auto quals = type->GetQualifiers()->GetQualfierGroup();
            // Precision Qualifier
            if (quals.GetHighp()) {
                result += "highp ";
            }
            if (quals.GetMediump()) {
                result += "mediump ";
            }
            if (quals.GetLowp()) {
                result += "lowp ";
            }

            // Storage/Parameter qualifiers
            if (quals.GetConst()) {
                result += "const ";
            }
            if (quals.GetIn()) {
                result += "in ";
            }
            if (quals.GetOut()) {
                result += "out ";
            }
            if (quals.GetInout()) {
                result += "inout ";
            }
            if (quals.GetAttribute()) {
                result += "attribute ";
            }
            if (quals.GetUniform()) {
                result += "uniform ";
            }
            if (quals.GetVarying()) {
                result += "varying ";
            }
            if (quals.GetBuffer()) {
                result += "buffer ";
            }
            if (quals.GetShared()) {
                result += "shared ";
            }

            // Auxiliary storage qualifiers
            if (quals.GetCentroid()) {
                result += "centroid ";
            }
            if (quals.GetSample()) {
                result += "sample ";
            }
            if (quals.GetPatch()) {
                result += "patch ";
            }

            // Interpolation qualifiers
            if (quals.qSmooth) {
                result += "smooth ";
            }
            if (quals.qFlat) {
                result += "flat ";
            }
            if (quals.qNoperspective) {
                result += "noperspective ";
            }

            // Variance qualifier
            if (quals.qInvariant) {
                result += "invariant ";
            }

            // Precise qualifier
            if (quals.qPrecise) {
                result += "precise ";
            }

            // Memory qualifiers
            if (quals.qCoherent) {
                result += "coherent ";
            }
            if (quals.qCoherent) {
                result += "coherent ";
            }
            if (quals.qVolatile) {
                result += "volatile ";
            }
            if (quals.qRestrict) {
                result += "restrict ";
            }
            if (quals.qReadonly) {
                result += "readonly ";
            }
            if (quals.qWriteonly) {
                result += "writeonly ";
            }
        }

        // FIXME: reconstruct from TypeDesc
        if (auto structDecl = type->GetStructDecl()) {
            result += "struct ";
            if (structDecl->GetDeclToken()) {
                result += ReconstructSourceText(*structDecl->GetDeclToken());
            }
            result += " { ... }";
        }
        else {
            result += ReconstructSourceText(type->GetTypeNameTok());
        }

        if (auto typeDesc = type->GetTypeDesc()->GetArrayDesc()) {
            // FIXME: print dim sizes in correct order
            for (auto dimSize : typeDesc->dimSizes) {
                if (dimSize == 0) {
                    result += "[]";
                }
                else {
                    result += fmt::format("[{}]", dimSize);
                }
            }
        }

        return result;
    }
    inline auto ReconstructSourceText(ArrayView<AstParamDecl*> params) -> std::string
    {
        std::string result;
        result += "(";
        for (auto param : params) {
            result += ReconstructSourceText(param->GetType());
            if (param->GetDeclToken()) {
                result += " ";
                result += ReconstructSourceText(*param->GetDeclToken());
            }
            result += ",";
        }
        if (result.ends_with(',')) {
            result.pop_back();
        }
        result += ")";

        return result;
    }

    inline auto ReconstructSourceText(std::string& buffer, AstQualType& type) -> void
    {
        // Reconstruct qualfiers
        if (type.GetQualifiers()) {
            auto quals = type.GetQualifiers()->GetQualfierGroup();
            // Precision Qualifier
            if (quals.GetHighp()) {
                buffer += "highp ";
            }
            if (quals.GetMediump()) {
                buffer += "mediump ";
            }
            if (quals.GetLowp()) {
                buffer += "lowp ";
            }

            // Storage/Parameter qualifiers
            if (quals.GetConst()) {
                buffer += "const ";
            }
            if (quals.GetIn()) {
                buffer += "in ";
            }
            if (quals.GetOut()) {
                buffer += "out ";
            }
            if (quals.GetInout()) {
                buffer += "inout ";
            }
            if (quals.GetAttribute()) {
                buffer += "attribute ";
            }
            if (quals.GetUniform()) {
                buffer += "uniform ";
            }
            if (quals.GetVarying()) {
                buffer += "varying ";
            }
            if (quals.GetBuffer()) {
                buffer += "buffer ";
            }
            if (quals.GetShared()) {
                buffer += "shared ";
            }

            // Auxiliary storage qualifiers
            if (quals.GetCentroid()) {
                buffer += "centroid ";
            }
            if (quals.GetSample()) {
                buffer += "sample ";
            }
            if (quals.GetPatch()) {
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
                buffer += ReconstructSourceText(*structDecl->GetDeclToken());
            }
            buffer += " { ... }";
        }
        else {
            buffer += ReconstructSourceText(type.GetTypeNameTok());
        }

        if (auto typeDesc = type.GetTypeDesc()->GetArrayDesc()) {
            // FIXME: print dim sizes in correct order
            for (auto dimSize : typeDesc->dimSizes) {
                if (dimSize == 0) {
                    buffer += "[]";
                }
                else {
                    buffer += fmt::format("[{}]", dimSize);
                }
            }
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, AstStructMemberDecl& decl, size_t index) -> void
    {
        // FIXME: this is exactly the same implementation with AstVariableDecl
        GLSLD_ASSERT(index < decl.GetDeclarators().size());

        buffer += ReconstructSourceText(decl.GetType());
        buffer += " ";

        const auto& declarator = decl.GetDeclarators()[index];
        buffer += declarator.declTok.text.StrView();
        if (declarator.arraySize) {
            for (auto dimSizeExpr : declarator.arraySize->GetSizeList()) {
                if (dimSizeExpr && dimSizeExpr->GetConstValue().GetValueType() == ConstValueType::Int32) {
                    buffer += fmt::format("[{}]", dimSizeExpr->GetConstValue().GetIntValue());
                }
                else {
                    buffer += "[]";
                }
            }
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, AstVariableDecl& decl, size_t index) -> void
    {
        GLSLD_ASSERT(index < decl.GetDeclarators().size());

        buffer += ReconstructSourceText(decl.GetType());
        buffer += " ";

        const auto& declarator = decl.GetDeclarators()[index];
        buffer += declarator.declTok.text.StrView();
        if (declarator.arraySize) {
            for (auto dimSizeExpr : declarator.arraySize->GetSizeList()) {
                if (dimSizeExpr && dimSizeExpr->GetConstValue().GetValueType() == ConstValueType::Int32) {
                    buffer += fmt::format("[{}]", dimSizeExpr->GetConstValue().GetIntValue());
                }
                else {
                    buffer += "[]";
                }
            }
        }
    }
    inline auto ReconstructSourceText(std::string& buffer, AstParamDecl& decl) -> void
    {
        buffer += ReconstructSourceText(decl.GetType());
        if (decl.GetDeclToken()) {
            buffer += " ";
            buffer += decl.GetDeclToken()->text.StrView();
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

} // namespace glsld