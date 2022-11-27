#pragma once
#include "SyntaxToken.h"
#include "Tokenizer.h"

#include <string>
#include <vector>
#include <unordered_set>

namespace glsld
{
    // This class manages everything about the source texts and tokens
    // FIXME: support token stream other than std::string
    // FIXME: support dynamic lexing
    class LexContext
    {
    public:
        LexContext(std::string sourceText) : sourceText(std::move(sourceText))
        {
            Initialize();
        }

        auto GetToken(size_t tokIndex) const -> SyntaxToken
        {
            return GetTokenRef(tokIndex);
        }

        auto GetSyntaxRange(size_t tokIndex) const -> SyntaxRange
        {
            return GetTokenRef(tokIndex).range;
        }

        //
        auto GetSyntaxRange(size_t beginTokIndex, size_t endTokIndex) const -> SyntaxRange
        {
            GLSLD_ASSERT(beginTokIndex <= endTokIndex);
            if (beginTokIndex == endTokIndex) {
                return SyntaxRange{
                    .begin = GetTokenRef(beginTokIndex).range.begin,
                    .end   = GetTokenRef(beginTokIndex).range.begin,
                };
            }
            else {
                return SyntaxRange{
                    .begin = GetTokenRef(beginTokIndex).range.begin,
                    .end   = GetTokenRef(endTokIndex - 1).range.end,
                };
            }
        }

        auto LookupSyntaxLocation(SyntaxLocation location) const -> SyntaxLocationInfo
        {
            if (location.GetIndex() < 0 || location.GetIndex() > locationInfo.size()) {
                return SyntaxLocationInfo{
                    .file   = -1,
                    .offset = -1,
                    .line   = -1,
                    .column = -1,
                };
            }

            return locationInfo[location.GetIndex()];
        }

    private:
        auto GetTokenRef(size_t tokIndex) const -> const SyntaxToken&
        {
            if (tokIndex < tokens.size()) {
                return tokens[tokIndex];
            }
            else {
                // This is EOF
                return tokens.back();
            }
        }

        auto Initialize() -> void
        {
            auto RegisterLocation = [this](int offset, int line, int column) -> SyntaxLocation {
                locationInfo.push_back(SyntaxLocationInfo{
                    .file   = 0,
                    .offset = offset,
                    .line   = line,
                    .column = column,
                });

                return SyntaxLocation{static_cast<int>(locationInfo.size() - 1)};
            };

            glsld::Tokenizer tokenizer{sourceText};
            std::string buffer;
            while (true) {
                buffer.clear();
                auto tokInfo = tokenizer.NextToken(buffer);

                LexString tokText;
                if (auto it = atomTable.find(buffer); it != atomTable.end()) {
                    tokText = LexString{it->c_str()};
                }
                else {
                    tokText = LexString{atomTable.insert(buffer).first->c_str()};
                }

                tokens.push_back(SyntaxToken{
                    .klass = tokInfo.klass,
                    .text  = tokText,
                    .range =
                        {
                            .begin = RegisterLocation(tokInfo.rawOffset, tokInfo.lineBegin, tokInfo.columnBegin),
                            .end   = RegisterLocation(tokInfo.rawOffset + tokInfo.rawSize, tokInfo.lineEnd,
                                                      tokInfo.columnEnd),
                        },
                });

                if (tokInfo.klass == glsld::TokenKlass::Eof) {
                    break;
                }
            }
        }

        std::string sourceText;

        // FIXME: optimize memory layout
        std::unordered_set<std::string> atomTable;

        std::vector<SyntaxToken> tokens;
        std::vector<SyntaxLocationInfo> locationInfo;
    };
} // namespace glsld