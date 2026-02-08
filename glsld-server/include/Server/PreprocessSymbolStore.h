#pragma once
#include "Compiler/SyntaxToken.h"
#include "Compiler/PPCallback.h"

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

    enum class PPMacroOccurrenceType
    {
        Define,
        Undef,
        IfDef,
        Expand,
    };

    struct PPMacroSymbol
    {
        PPToken macroName;
        AstSyntaxRange expandedTokens;
        const PPMacroDefinition* definition = nullptr;
        PPMacroOccurrenceType occurrenceType;
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

    struct PPInactiveRegion
    {
        // 0-based line of which the inactive region starts
        int startLine;

        // 0-based line of which the inactive region ends (aka. the first active line after the inactive region)
        int endLine;
    };

    // TODO: collect all header name occurrences, including the path to the header file
    // TODO: collect all spelled macro uses in the main file, including the definition and expansion?
    // TODO: 1) support basic hover on macro, showing name only. 2) support definition tokens 3) support expansion
    class PreprocessInfoStore
    {
    private:
        std::vector<std::unique_ptr<PPMacroDefinition>> macroDefinitions;
        std::vector<PPSymbolOccurrence> occurrences;
        std::vector<PPInactiveRegion> inactiveRegions;

    public:
        auto GetCollectionCallback() -> std::unique_ptr<PPCallback>;

        auto GetAllOccurrences() const -> ArrayView<PPSymbolOccurrence>
        {
            return occurrences;
        }

        auto GetInactiveRegions() const -> ArrayView<PPInactiveRegion>
        {
            return inactiveRegions;
        }

        auto QueryPPSymbol(TextPosition position) const -> const PPSymbolOccurrence*;
    };

} // namespace glsld