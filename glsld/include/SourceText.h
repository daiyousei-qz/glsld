#pragma once
#include "Common.h"
#include "Protocol.h"
#include <iterator>
#include <span>
#include <vector>
#include <string_view>
#include <optional>

namespace glsld
{
    struct TextPosition
    {
        int line      = 0;
        int character = 0;

        static auto FromLspPosition(lsp::Position position) -> TextPosition
        {
            return TextPosition{
                .line      = static_cast<int>(position.line),
                .character = static_cast<int>(position.character),
            };
        }
        static auto ToLspPosition(TextPosition position) -> lsp::Position
        {
            return lsp::Position{
                .line      = static_cast<uint32_t>(position.line),
                .character = static_cast<uint32_t>(position.character),
            };
        }
    };

    struct TextRange
    {
        TextPosition start;
        TextPosition end;

        auto ContainPosition(TextPosition position) -> bool
        {
            if (position.line < start.line || (position.line == start.line && position.character < start.character)) {
                return false;
            }
            if (position.line > end.line || (position.line == end.line && position.character >= end.character)) {
                return false;
            }

            return true;
        }

        static auto FromLspRange(lsp::Range range) -> TextRange
        {
            return TextRange{
                .start = TextPosition::FromLspPosition(range.start),
                .end   = TextPosition::FromLspPosition(range.end),
            };
        }
        static auto ToLspRange(TextRange range) -> lsp::Range
        {
            return lsp::Range{
                .start = TextPosition::ToLspPosition(range.start),
                .end   = TextPosition::ToLspPosition(range.end),
            };
        }
    };

    struct SourcePiece
    {
        std::optional<TextRange> range;
        std::string_view text;
    };

    // inline auto ContainPosition(TextRange range, TextPosition position) -> bool
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
    //         if (!ContainPosition(TextRange{.start = startPosition, .end = *endPosition}, midPosition)) {
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

} // namespace glsld