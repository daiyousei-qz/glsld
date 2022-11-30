#pragma once
#include "Common.h"
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
    };

    struct TextRange
    {
        TextPosition start;
        TextPosition end;
    };

    struct SourcePiece
    {
        std::optional<TextRange> range;
        std::string_view text;
    };

    inline auto ContainPosition(TextRange range, TextPosition position) -> bool
    {
        if (position.line < range.start.line ||
            (position.line == range.start.line && position.character < range.start.character)) {
            return false;
        }
        if (position.line > range.end.line ||
            (position.line == range.end.line && position.character >= range.end.character)) {
            return false;
        }

        return true;
    }

    inline auto ComputeEndPosition(std::string_view text, TextPosition pieceStart)
    {
        TextPosition cur = pieceStart;
        for (auto it = text.begin(); it != text.end(); ++it) {
            auto ch = *it;
            if (ch == '\n') {
                cur.line += 1;
                cur.character = 0;
            }
            else {
                cur.character += 1;
            }
        }

        return cur;
    }

    inline auto SplitSourcePiece(std::string_view text, TextPosition startPosition, TextPosition midPosition,
                                 std::optional<TextPosition> endPosition)
        -> std::tuple<TextPosition, std::string_view, std::string_view>
    {
        if (endPosition) {
            if (!ContainPosition(TextRange{.start = startPosition, .end = *endPosition}, midPosition)) {
                return {*endPosition, text, std::string_view{}};
            }
        }

        TextPosition cur = startPosition;
        for (auto it = text.begin(); it != text.end(); ++it) {
            auto ch = *it;
            if (cur.line >= midPosition.line && cur.character >= midPosition.character) {
                return {cur, std::string_view{text.begin(), it}, std::string_view{it, text.end()}};
            }

            if (ch == '\n') {
                cur.line += 1;
                cur.character = 0;
            }
            else {
                cur.character += 1;
            }
        }

        return {cur, text, std::string_view{}};
    }

    inline auto ApplyTextChange(std::span<SourcePiece const> currentSources, TextRange changeRange,
                                std::string_view text) -> std::vector<SourcePiece>
    {
        std::vector<SourcePiece> result;
        bool enteredChangeRange = false;
        bool exitedChangeRange  = false;
        for (const auto& piece : currentSources) {
            if (exitedChangeRange) {
                result.push_back(SourcePiece{
                    .range = std::nullopt,
                    .text  = piece.text,
                });
            }
            else {
                std::string_view currentPieceText = piece.text;

                TextPosition currentPieceStart = {};
                if (!result.empty()) {
                    GLSLD_ASSERT(result.back().range.has_value());
                    currentPieceStart = result.back().range->end;
                }

                std::optional<TextPosition> currentPieceEnd = std::nullopt;
                if (piece.range) {
                    currentPieceEnd = piece.range->end;
                }

                if (!enteredChangeRange) {
                    auto [posEnd, lhs, rhs] =
                        SplitSourcePiece(currentPieceText, currentPieceStart, changeRange.start, currentPieceEnd);

                    if (!lhs.empty()) {
                        // lhs holds everything before the start
                        result.push_back(SourcePiece{
                            .range = TextRange{.start = currentPieceStart, .end = posEnd},
                            .text  = lhs,
                        });
                    }

                    if (!rhs.empty()) {
                        // we've find the start
                        enteredChangeRange = true;

                        // update current piece info so we could continue finding end position
                        currentPieceText  = rhs;
                        currentPieceStart = posEnd;
                    }
                }

                if (enteredChangeRange) {
                    auto [posEnd, lhs, rhs] =
                        SplitSourcePiece(currentPieceText, currentPieceStart, changeRange.end, currentPieceEnd);

                    if (!rhs.empty()) {
                        // we've find the end
                        result.push_back(SourcePiece{
                            .range = std::nullopt,
                            .text  = text,
                        });

                        result.push_back(SourcePiece{
                            .range = std::nullopt,
                            .text  = rhs,
                        });

                        exitedChangeRange = true;
                    }
                }
            }
        }

        if (!enteredChangeRange) {
            result.push_back(SourcePiece{
                .range = std::nullopt,
                .text  = text,
            });
        }

        return result;
    }

    inline auto ToString(std::span<SourcePiece const> currentSources) -> std::string
    {
        std::string result;
        for (const auto& piece : currentSources) {
            result += piece.text;
        }

        return result;
    }
} // namespace glsld