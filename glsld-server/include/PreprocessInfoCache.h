#pragma once
#include "Compiler/SyntaxToken.h"
#include "Compiler/PPCallback.h"

#include <algorithm>
#include <memory>
#include <variant>
#include <vector>

namespace glsld
{
    struct CachedHeaderName
    {
        std::string headerName;
        std::string headerAbsolutePath;
    };

    struct CachedMacroDefinition
    {
        PPToken macroName;
        std::vector<PPToken> params;
        std::vector<PPToken> tokens;
        bool isFunctionLike;
    };

    struct CachedMacroUsage
    {
        PPToken macroName;
        AstSyntaxRange expandedTokens;
        const CachedMacroDefinition* definition;
    };

    class PPSymbolOccurrence
    {
    private:
        TextRange spelledRange;

        using DataVariant = std::variant<CachedHeaderName, CachedMacroDefinition, CachedMacroUsage>;
        std::unique_ptr<DataVariant> data;

    public:
        PPSymbolOccurrence(TextRange spelledRange, CachedHeaderName headerName)
            : spelledRange(spelledRange), data(std::make_unique<DataVariant>(std::move(headerName)))
        {
        }

        PPSymbolOccurrence(TextRange spelledRange, CachedMacroDefinition macroDefinition)
            : spelledRange(spelledRange), data(std::make_unique<DataVariant>(std::move(macroDefinition)))
        {
        }

        PPSymbolOccurrence(TextRange spelledRange, CachedMacroUsage macroUsage)
            : spelledRange(spelledRange), data(std::make_unique<DataVariant>(std::move(macroUsage)))
        {
        }

        auto GetSpelledRange() const -> TextRange
        {
            return spelledRange;
        }

        auto GetHeaderNameInfo() const -> const CachedHeaderName*
        {
            return std::get_if<CachedHeaderName>(&*data);
        }

        auto GetMacroDefinitionInfo() const -> const CachedMacroDefinition*
        {
            return std::get_if<CachedMacroDefinition>(&*data);
        }

        auto GetMacroUsageInfo() const -> const CachedMacroUsage*
        {
            return std::get_if<CachedMacroUsage>(&*data);
        }
    };

    // TODO: collect all header name occurrences, including the path to the header file
    // TODO: collect all spelled macro uses in the main file, including the definition and expansion?
    // TODO: 1) support basic hover on macro, showing name only. 2) support definition tokens 3) support expansion
    class PreprocessInfoCache
    {
    private:
        std::vector<PPToken> headerNames;
        std::vector<PPToken> macroUses;

        std::vector<PPSymbolOccurrence> occurrences;

    public:
        auto GetCollectionCallback() -> std::unique_ptr<PPCallback>
        {
            class PreprocessInfoCollector final : public PPCallback
            {
            private:
                PreprocessInfoCache& cache;

                int includeDepth = 0;
                std::unordered_map<std::string, const CachedMacroDefinition*> macroDefinitions;

            public:
                PreprocessInfoCollector(PreprocessInfoCache& cache) : cache(cache)
                {
                }

                virtual auto OnIncludeDirective(const PPToken& headerName, StringView resolvedPath) -> void override
                {
                    if (includeDepth == 0) {
                        cache.headerNames.push_back(headerName);
                        cache.occurrences.push_back(PPSymbolOccurrence{
                            headerName.spelledRange, CachedHeaderName{headerName.text.Str(), resolvedPath.Str()}});
                    }
                }
                virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params,
                                               ArrayView<PPToken> tokens, bool isFunctionLike) -> void override
                {
                    if (includeDepth == 0) {
                        cache.macroUses.push_back(macroName);
                        cache.occurrences.push_back(PPSymbolOccurrence{
                            macroName.spelledRange,
                            CachedMacroDefinition{macroName, std::vector<PPToken>(params.begin(), params.end()),
                                                  std::vector<PPToken>(tokens.begin(), tokens.end()), isFunctionLike}});
                    }
                }
                virtual auto OnUndefDirective(const PPToken& macroName) -> void override
                {
                    if (includeDepth == 0) {
                        cache.macroUses.push_back(macroName);
                    }
                }
                virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void override
                {
                    if (includeDepth == 0) {
                        cache.macroUses.push_back(macroName);
                    }
                }

                virtual auto OnEnterIncludedFile() -> void override
                {
                    includeDepth += 1;
                }
                virtual auto OnExitIncludedFile() -> void override
                {
                    includeDepth -= 1;
                }
                virtual auto OnMacroExpansion(const PPToken& macroUse, AstSyntaxRange expansionRange) -> void override
                {
                    if (includeDepth == 0 && !macroUse.spelledRange.IsEmpty()) {
                        cache.macroUses.push_back(macroUse);
                        cache.occurrences.push_back(PPSymbolOccurrence{
                            macroUse.spelledRange, CachedMacroUsage{macroUse, expansionRange, nullptr}});
                    }
                }
            };

            return std::make_unique<PreprocessInfoCollector>(*this);
        }

        auto GetHeaderNames() const -> ArrayView<PPToken>
        {
            return headerNames;
        }

        auto GetMacroUses() const -> ArrayView<PPToken>
        {
            return macroUses;
        }

        auto GetInterestingOccurrences() const -> ArrayView<PPSymbolOccurrence>
        {
            return occurrences;
        }

        auto FindPPSymbolOccurrence(TextPosition position) const -> const PPSymbolOccurrence*
        {
            auto it = std::ranges::upper_bound(
                occurrences, position, std::ranges::less{},
                [](const PPSymbolOccurrence& occurence) { return occurence.GetSpelledRange().end; });
            if (it != occurrences.end() && it->GetSpelledRange().Contains(position)) {
                return &*it;
            }

            return nullptr;
        }
    };

} // namespace glsld