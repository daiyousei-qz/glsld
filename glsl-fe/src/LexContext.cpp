#include "LexContext.h"

namespace glsld
{
    namespace
    {
        auto GetLanguageAtomTable() -> const AtomTable&
        {
            static AtomTable languageAtomTable = []() {
                AtomTable result;

#define DECL_KEYWORD(KEYWORD) result.GetAtom(#KEYWORD);
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

                return result;
            }();

            return languageAtomTable;
        }

        // Our lexer doesn't know about GLSL keywords and treats them as identifiers.
        // So we have to fix the token class before adding it to the LexContext.
        auto FixKeywordTokenKlass(TokenKlass klass, AtomString text) -> TokenKlass
        {
            static std::unordered_map<AtomString, TokenKlass> keywordLookup = []() {
                std::unordered_map<AtomString, TokenKlass> result;

#define DECL_KEYWORD(KEYWORD) result[GetLanguageAtomTable().GetAtom(#KEYWORD)] = TokenKlass::K_##KEYWORD;
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

                return result;
            }();

            if (klass == TokenKlass::Identifier) {
                if (auto it = keywordLookup.find(text); it != keywordLookup.end()) {
                    return it->second;
                }
            }

            return klass;
        }

    } // namespace

    LexContext::LexContext()
    {
        atomTable.Import(GetLanguageAtomTable());
    }
    LexContext::~LexContext()
    {
    }

    auto LexContext::AddToken(FileID fileID, const PPToken& token) -> void
    {
        GLSLD_ASSERT(tokens.empty() || tokens.back().klass != TokenKlass::Eof);

        if (token.klass != TokenKlass::Comment) {
            tokens.push_back(RawSyntaxToken{
                .file  = fileID,
                .klass = FixKeywordTokenKlass(token.klass, token.text),
                .text  = token.text,
                .range = token.range,
            });
        }
    }

    auto LexContext::GetToken(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        GLSLD_ASSERT(tokIndex < tokens.size());
        return SyntaxToken{
            .index = tokIndex,
            .klass = tokens[tokIndex].klass,
            .text  = tokens[tokIndex].text,
        };
    }

    auto LexContext::GetTokenOrEof(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        if (tokIndex >= tokens.size()) {
            return GetToken(tokens.size() - 1);
        }
        else {
            return GetToken(tokIndex);
        }
    }

    auto LexContext::FindTokenByTextPosition(TextPosition position) const -> SyntaxToken
    {
        auto it =
            std::ranges::lower_bound(tokens, position, {}, [](const RawSyntaxToken& tok) { return tok.range.start; });
        if (it != tokens.end() && it != tokens.begin()) {
            SyntaxTokenIndex index = std::distance(tokens.begin(), it) - 1;
            return SyntaxToken{
                .index = index,
                .klass = tokens[index].klass,
                .text  = tokens[index].text,
            };
        }
        else {
            return SyntaxToken{};
        }
    }

    auto LexContext::LookupTextRange(SyntaxTokenIndex tokIndex) const -> TextRange
    {
        GLSLD_ASSERT(tokIndex < tokens.size());
        return tokens[tokIndex].range;
    }

    auto LexContext::LookupTextRange(SyntaxToken token) const -> TextRange
    {
        if (token.IsValid()) {
            GLSLD_ASSERT(token.index < tokens.size());
            return tokens[token.index].range;
        }
        else {
            return TextRange{};
        }
    }

    auto LexContext::LookupTextRange(SyntaxTokenRange range) const -> TextRange
    {
        GLSLD_ASSERT(range.endTokenIndex < tokens.size());
        if (range.endTokenIndex > range.startTokenIndex) {
            return TextRange{
                tokens[range.startTokenIndex].range.start,
                tokens[range.endTokenIndex - 1].range.end,
            };
        }
        else {
            return TextRange{
                tokens[range.startTokenIndex].range.start,
                tokens[range.startTokenIndex].range.start,
            };
        }
    }
} // namespace glsld
