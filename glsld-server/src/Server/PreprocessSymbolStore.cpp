#include "Server/PreprocessSymbolStore.h"

#include "Basic/StringMap.h"

#include <algorithm>

namespace glsld
{
    auto PreprocessSymbolStore::GetCollectionCallback() -> std::unique_ptr<PPCallback>
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
                    store.occurrences.push_back(PPSymbolOccurrence{headerName.spelledRange,
                                                                   PPHeaderNameSymbol{headerName, resolvedPath.Str()}});
                }
            }
            virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params,
                                           ArrayView<PPToken> tokens, bool isFunctionLike) -> void override
            {
                if (includeDepth == 0) {
                    auto macroDefinition = DefineMacro(macroName, params, tokens, isFunctionLike);
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{macroName, {}, macroDefinition, PPMacroOccurrenceType::Define}});
                }
            }
            virtual auto OnUndefDirective(const PPToken& macroName) -> void override
            {
                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{
                            macroName, {}, FindMacro(macroName.text.StrView()), PPMacroOccurrenceType::Undef}});
                    UndefMacro(macroName);
                }
            }
            virtual auto OnIfDefDirective(const PPToken& macroName, bool isNDef) -> void override
            {
                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{
                            macroName, {}, FindMacro(macroName.text.StrView()), PPMacroOccurrenceType::IfDef}});
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
            virtual auto OnMacroExpansion(const PPToken& macroNameTok, AstSyntaxRange expansionRange) -> void override
            {
                if (includeDepth == 0 && !macroNameTok.spelledRange.IsEmpty()) {
                    store.occurrences.push_back(PPSymbolOccurrence{macroNameTok.spelledRange,
                                                                   PPMacroSymbol{macroNameTok, expansionRange,
                                                                                 FindMacro(macroNameTok.text.StrView()),
                                                                                 PPMacroOccurrenceType::Expand}});
                }
            }
        };

        return std::make_unique<PreprocessInfoCollector>(*this);
    }

    auto PreprocessSymbolStore::QueryPPSymbol(TextPosition position) const -> const PPSymbolOccurrence*
    {
        auto it = std::ranges::upper_bound(
            occurrences, position, std::ranges::less{},
            [](const PPSymbolOccurrence& occurence) { return occurence.GetSpelledRange().end; });
        if (it != occurrences.end() && it->GetSpelledRange().Contains(position)) {
            return &*it;
        }

        return nullptr;
    }
} // namespace glsld