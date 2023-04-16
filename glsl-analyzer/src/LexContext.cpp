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

    auto LexContext::AddToken(const PPToken& token, TextRange expandedRange) -> void
    {
        GLSLD_ASSERT(tokens.empty() || tokens.back().klass != TokenKlass::Eof);

        if (token.klass != TokenKlass::Comment) {
            tokens.push_back(RawSyntaxTokenInfo{
                .klass         = FixKeywordTokenKlass(token.klass, token.text),
                .spelledFile   = token.spelledFile,
                .spelledRange  = token.spelledRange,
                .expandedRange = expandedRange,
                .text          = token.text,
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

    auto LexContext::GetTokenSafe(SyntaxTokenIndex tokIndex) const -> SyntaxToken
    {
        if (tokIndex < tokens.size()) {
            return GetToken(tokIndex);
        }
        else {
            return GetToken(GetTokenCount() - 1);
        }
    }

    auto LexContext::FindTokenByTextPosition(TextPosition position) const -> SyntaxToken
    {
        auto it = std::ranges::lower_bound(tokens, position, {},
                                           [](const RawSyntaxTokenInfo& tok) { return tok.expandedRange.start; });
        if (it != tokens.end() && it != tokens.begin()) {
            SyntaxTokenIndex index = std::distance(tokens.begin(), it) - 1;
            return SyntaxToken{
                .index = index,
                .klass = tokens[index].klass,
                .text  = tokens[index].text,
            };
        }
        else {
            return GetToken(GetTokenCount() - 1);
        }
    }
} // namespace glsld
