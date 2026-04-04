#include "Server/PreprocessSymbolStore.h"

#include "Support/StringMap.h"

#include <algorithm>

namespace glsld
{
    auto PreprocessInfoStore::CreateCollectionCallback(const MacroTable* preambleMacroTable)
        -> std::unique_ptr<PPCallback>
    {
        class PreprocessInfoCollector final : public PPCallback
        {
        private:
            PreprocessInfoStore& store;

            // We only collect PP symbol occurrences in the main file, so we need to track the include depth here.
            int includeDepth = 0;

            // This tracks the start line of the currently active inactive region. When we encounter a branch directive
            // that makes the current region inactive, we set this to the start line of the inactive region. When we
            // encounter a branch directive that makes the current region active again, we push the inactive region to
            // the store and reset this.
            std::optional<int> inactiveRegionStartLine;

            // This tracks the currently defined macros as we scan through the translation unit.
            // The macro definitions are either from:
            // - The preamble macro table
            // - The macro definition buffer in the store
            UnorderedStringMap<const MacroDefinition*> macroLookup;

            auto FindMacro(StringView macroName) -> const MacroDefinition*
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
            PreprocessInfoCollector(PreprocessInfoStore& cache, const MacroTable* preambleMacroTable) : store(cache)
            {
                if (preambleMacroTable) {
                    for (const auto& macroDef : preambleMacroTable->GetMacroDefinitions()) {
                        macroLookup[macroDef.defToken.text.StrView()] = &macroDef;
                    }
                }
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
                // While macros in included files are not relevant to user interaction, we still need record them for
                // later queries.
                store.macroDefinitions.push_back(MacroDefinition{
                    .isCompilerDefined = false,
                    .isFunctionLike    = isFunctionLike,
                    .defToken          = macroName,
                    .paramTokens       = std::vector<PPToken>(paramTokens.begin(), paramTokens.end()),
                    .expansionTokens   = std::vector<PPToken>(replacementTokens.begin(), replacementTokens.end()),
                });
                macroLookup[macroName.text.StrView()] = &store.macroDefinitions.back();

                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{macroName, {}, &store.macroDefinitions.back(), PPMacroOccurrenceType::Define}});
                }
            }
            auto OnUndefDirective(ArrayView<PPToken> tokens, const PPToken& macroName) -> void override
            {
                if (includeDepth == 0) {
                    store.occurrences.push_back(PPSymbolOccurrence{
                        macroName.spelledRange,
                        PPMacroSymbol{
                            macroName, {}, FindMacro(macroName.text.StrView()), PPMacroOccurrenceType::Undef}});
                }

                // While macros in included files are not relevant to user interaction, we still need record them for
                // later queries.
                macroLookup.Erase(macroName.text.StrView());
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

        return std::make_unique<PreprocessInfoCollector>(*this, preambleMacroTable);
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