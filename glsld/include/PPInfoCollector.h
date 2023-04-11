#pragma once
#include "SyntaxToken.h"
#include "PPCallback.h"

#include <vector>

namespace glsld
{
    class PPInfoCache
    {
    public:
        auto GetHeaderNames() const -> ArrayView<PPToken>
        {
            return headerNames;
        }

        auto GetMacroUses() const -> ArrayView<PPToken>
        {
            return macroUses;
        }

    protected:
        std::vector<PPToken> headerNames;
        std::vector<PPToken> macroUses;
    };

    class PPInfoCollector : public PPInfoCache, public PPCallback
    {
    public:
        virtual auto OnIncludeDirective(const PPToken& headerName) -> void override
        {
            headerNames.push_back(headerName);
        }
        virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params) -> void override
        {
            macroUses.push_back(macroName);
        }
        virtual auto OnUndefDirective(const PPToken& macroName) -> void override
        {
            macroUses.push_back(macroName);
        }

        virtual auto OnEnterIncludedFile() -> void override
        {
            includeDepth += 1;
        }
        virtual auto OnExitIncludedFile() -> void override
        {
            includeDepth -= 1;
        }
        virtual auto OnMacroExpansion(const PPToken& macroUse) -> void override
        {
            if (includeDepth == 0 && !macroUse.spelledRange.IsEmpty()) {
                macroUses.push_back(macroUse);
            }
        }

    private:
        int includeDepth = 0;
    };
} // namespace glsld