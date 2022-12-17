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

    struct SourcePiece
    {
        std::optional<TextRange> range;
        std::string_view text;
    };

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