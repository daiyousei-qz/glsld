#include "Server/PreprocessSymbolStore.h"

#include "Support/StringMap.h"

#include <algorithm>

namespace glsld
{
    auto PreprocessInfoStore::GetCollectionCallback() -> std::unique_ptr<PPCallback>
    {
        class PreprocessInfoCollector final : public PPCallback
        {
        private:
            PreprocessInfoStore& store;

            int includeDepth = 0;
            std::optional<int> inactiveRegionStartLine;
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

            auto EnterInactiveRange(int startLine) -> void
            {
                inactiveRegionStartLine = startLine;
            }

            auto TryLeaveInactiveRange(int endLine) -> void
            {
                if (inactiveRegionStartLine) {
                    store.inactiveRegions.push_back(PPInactiveRegion{*inactiveRegionStartLine, endLine});
                    inactiveRegionStartLine = std::nullopt;
                }
            }

        public:
            PreprocessInfoCollector(PreprocessInfoStore& cache) : store(cache)
            {
            }

            auto OnIncludeDirective(ArrayView<PPToken> tokens, const PPToken& headerName, StringView resolvedPath)
                -> void override
            {
                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{headerName.spelledRange,
                                                                   PPHeaderNameSymbol{headerName, resolvedPath.Str()}});
                }
            }
            auto OnDefineDirective(ArrayView<PPToken> tokens, const PPToken& macroName, ArrayView<PPToken> paramTokens,
                                   ArrayView<PPToken> replacementTokens, bool isFunctionLike) -> void override
            {
                if (includeDepth == 0) {
                    auto macroDefinition = DefineMacro(macroName, paramTokens, replacementTokens, isFunctionLike);
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{macroName, {}, macroDefinition, PPMacroOccurrenceType::Define}});
                }
            }
            auto OnUndefDirective(ArrayView<PPToken> tokens, const PPToken& macroName) -> void override
            {
                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{
                            macroName, {}, FindMacro(macroName.text.StrView()), PPMacroOccurrenceType::Undef}});
                    UndefMacro(macroName);
                }
            }
            auto OnIfDefDirective(ArrayView<PPToken> tokens, const PPToken& macroName, bool isNDef, bool isActive)
                -> void override
            {
                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{
                            macroName, {}, FindMacro(macroName.text.StrView()), PPMacroOccurrenceType::IfDef}});

                    if (!isActive) {
                        EnterInactiveRange(tokens.back().spelledRange.end.line + 1);
                    }
                }
            }
            auto OnIfDirective(ArrayView<PPToken> tokens, bool isActive) -> void override
            {
                if (includeDepth == 0) {
                    if (!isActive) {
                        EnterInactiveRange(tokens.back().spelledRange.end.line + 1);
                    }
                }
            }
            auto OnElifDirective(ArrayView<PPToken> tokens, bool isActive) -> void override
            {
                if (includeDepth == 0) {
                    TryLeaveInactiveRange(tokens.back().spelledRange.start.line);
                    if (!isActive) {
                        EnterInactiveRange(tokens.back().spelledRange.end.line + 1);
                    }
                }
            }
            auto OnElseDirective(ArrayView<PPToken> tokens, bool isActive) -> void override
            {
                if (includeDepth == 0) {
                    TryLeaveInactiveRange(tokens.back().spelledRange.start.line);
                    if (!isActive) {
                        EnterInactiveRange(tokens.back().spelledRange.end.line + 1);
                    }
                }
            }
            auto OnEndifDirective(ArrayView<PPToken> tokens) -> void override
            {
                if (includeDepth == 0) {
                    TryLeaveInactiveRange(tokens.back().spelledRange.start.line);
                }
            }

            auto OnEnterIncludedFile() -> void override
            {
                includeDepth += 1;
            }
            auto OnExitIncludedFile() -> void override
            {
                includeDepth -= 1;
            }
            auto OnMacroExpansion(const PPToken& macroNameTok, AstSyntaxRange expansionRange) -> void override
            {
                if (includeDepth == 0 && !macroNameTok.spelledRange.IsEmpty()) {
                    store.occurrences.push_back(PPSymbolOccurrence{macroNameTok.spelledRange,
                                                                   PPMacroSymbol{macroNameTok, expansionRange,
                                                                                 FindMacro(macroNameTok.text.StrView()),
                                                                                 PPMacroOccurrenceType::Expand}});
                }
            }
            auto OnDefinedOperator(const PPToken& macroNameTok, bool isDefined) -> void override
            {
                if (includeDepth == 0 && !macroNameTok.spelledRange.IsEmpty()) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroNameTok.spelledRange,
                        PPMacroSymbol{
                            macroNameTok, {}, FindMacro(macroNameTok.text.StrView()), PPMacroOccurrenceType::IfDef}});
                }
            }
        };

        return std::make_unique<PreprocessInfoCollector>(*this);
    }

    auto PreprocessInfoStore::QueryPPSymbol(TextPosition position) const -> const PPSymbolOccurrence*
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