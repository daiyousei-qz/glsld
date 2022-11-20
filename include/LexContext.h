#pragma once
#include "SyntaxTree.h"
#include "Tokenizer.h"

#include <vector>
#include <unordered_set>

namespace glsld
{
    class LexContext
    {
    public:
        LexContext(std::string sourceText) : sourceText(std::move(sourceText))
        {
            Initialize();
        }

        auto GetToken(size_t tokIndex) -> SyntaxToken
        {
            return GetTokenRef(tokIndex);
        }

        auto GetSyntaxRange(size_t tokIndex) -> SyntaxRange
        {
            return GetTokenRef(tokIndex).range;
        }

        //
        auto GetSyntaxRange(size_t beginTokIndex, size_t endTokIndex) -> SyntaxRange
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

    private:
        auto GetTokenRef(size_t tokIndex) -> const SyntaxToken&
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

                std::string_view tokText;
                if (auto it = atomTable.find(buffer); it != atomTable.end()) {
                    tokText = *it;
                }
                else {
                    tokText = *atomTable.insert(buffer).first;
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

        std::unordered_set<std::string> atomTable;

        std::vector<SyntaxToken> tokens;
        std::vector<SyntaxLocationInfo> locationInfo;
    };
} // namespace glsld