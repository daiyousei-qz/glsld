#pragma once
#include "Basic/StringMap.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/PPCallback.h"

#include <algorithm>
#include <memory>
#include <variant>
#include <vector>

namespace glsld
{
    struct PPMacroDefinition
    {
        PPToken macroName;
        std::vector<PPToken> params;
        std::vector<PPToken> tokens;
        bool isFunctionLike;
    };

    struct PPHeaderNameSymbol
    {
        PPToken headerName;
        std::string headerAbsolutePath;
    };

    struct PPMacroSymbol
    {
        PPToken macroName;
        AstSyntaxRange expandedTokens;
        const PPMacroDefinition* definition = nullptr;
    };

    class PPSymbolOccurrence
    {
    private:
        TextRange spelledRange;

        using DataVariant = std::variant<PPHeaderNameSymbol, PPMacroSymbol>;
        std::unique_ptr<DataVariant> data;

    public:
        PPSymbolOccurrence(TextRange spelledRange, PPHeaderNameSymbol headerName)
            : spelledRange(spelledRange), data(std::make_unique<DataVariant>(std::move(headerName)))
        {
        }

        PPSymbolOccurrence(TextRange spelledRange, PPMacroSymbol macroUsage)
            : spelledRange(spelledRange), data(std::make_unique<DataVariant>(std::move(macroUsage)))
        {
        }

        auto GetSpelledRange() const -> TextRange
        {
            return spelledRange;
        }

        auto GetHeaderNameInfo() const -> const PPHeaderNameSymbol*
        {
            return std::get_if<PPHeaderNameSymbol>(&*data);
        }

        auto GetMacroInfo() const -> const PPMacroSymbol*
        {
            return std::get_if<PPMacroSymbol>(&*data);
        }
    };

    // TODO: collect all header name occurrences, including the path to the header file
    // TODO: collect all spelled macro uses in the main file, including the definition and expansion?
    // TODO: 1) support basic hover on macro, showing name only. 2) support definition tokens 3) support expansion
    class PreprocessSymbolStore
    {
    private:
        std::vector<std::unique_ptr<PPMacroDefinition>> macroDefinitions;
        std::vector<PPSymbolOccurrence> occurrences;

    public:
        auto GetCollectionCallback() -> std::unique_ptr<PPCallback>
        {
            class PreprocessInfoCollector final : public PPCallback
            {
            private:
                PreprocessSymbolStore& store;

                int includeDepth = 0;
                UnorderedStringMap<const PPMacroDefinition*> macroLookup;

                auto DefineMacro(const PPToken& macroName, ArrayView<PPToken> params, ArrayView<PPToken> tokens,
                                 bool isFunctionLike) -> const PPMacroDefinition*
                {
                    store.macroDefinitions.push_back(std::make_unique<PPMacroDefinition>(
                        macroName, std::vector<PPToken>(params.begin(), params.end()),
                        std::vector<PPToken>(tokens.begin(), tokens.end()), isFunctionLike));

                    return macroLookup[macroName.text.StrView()] = store.macroDefinitions.back().get();
                }

                auto UndefMacro(const PPToken& macroName) -> void
                {
                    macroLookup.Erase(macroName.text.StrView());
                }

                auto FindMacro(StringView macroName) -> const PPMacroDefinition*
                {
                    if (auto it = macroLookup.Find(macroName); it != macroLookup.end()) {
                        return it->second;
                    }

                    return nullptr;
                }

            public:
                PreprocessInfoCollector(PreprocessSymbolStore& cache) : store(cache)
                {
                }

                virtual auto OnIncludeDirective(const PPToken& headerName, StringView resolvedPath) -> void override
                {
                    if (includeDepth == 0) {
                        store.occurrences.push_back(PPSymbolOccurrence{
                            headerName.spelledRange, PPHeaderNameSymbol{headerName, resolvedPath.Str()}});
                    }
                }
                virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params,
                                               ArrayView<PPToken> tokens, bool isFunctionLike) -> void override
                {
                    if (includeDepth == 0) {
                        auto macroDefinition = DefineMacro(macroName, params, tokens, isFunctionLike);
                        store.occurrences.push_back(
                            PPSymbolOccurrence{macroName.spelledRange, PPMacroSymbol{macroName, {}, macroDefinition}});
                    }
                }
                virtual auto OnUndefDirective(const PPToken& macroName) -> void override
                {
                    if (includeDepth == 0) {
                        UndefMacro(macroName);
                    }
                }
                virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void override
                {
                    if (includeDepth == 0) {
                        store.occurrences.push_back(PPSymbolOccurrence{
                            macroName.spelledRange, PPMacroSymbol{macroName, {}, FindMacro(macroName.text.StrView())}});
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
                virtual auto OnMacroExpansion(const PPToken& macroNameTok, AstSyntaxRange expansionRange)
                    -> void override
                {
                    if (includeDepth == 0 && !macroNameTok.spelledRange.IsEmpty()) {
                        store.occurrences.push_back(PPSymbolOccurrence{
                            macroNameTok.spelledRange,
                            PPMacroSymbol{macroNameTok, expansionRange, FindMacro(macroNameTok.text.StrView())}});
                    }
                }
            };

            return std::make_unique<PreprocessInfoCollector>(*this);
        }

        auto GetAllOccurrences() const -> ArrayView<PPSymbolOccurrence>
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