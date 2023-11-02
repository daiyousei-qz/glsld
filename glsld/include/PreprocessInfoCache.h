#pragma once
#include "Compiler/SyntaxToken.h"
#include "Compiler/PPCallback.h"

#include <vector>

namespace glsld
{
    class PreprocessInfoCache
    {
    public:
        auto GetCollectionCallback() -> std::unique_ptr<PPCallback>
        {
            class PreprocessInfoCollector : public PPCallback
            {
            public:
                PreprocessInfoCollector(PreprocessInfoCache& cache) : cache(cache)
                {
                }

                virtual auto OnIncludeDirective(const PPToken& headerName) -> void override
                {
                    if (includeDepth == 0) {
                        cache.headerNames.push_back(headerName);
                    }
                }
                virtual auto OnDefineDirective(const PPToken& macroName, ArrayView<PPToken> params) -> void override
                {
                    if (includeDepth == 0) {
                        cache.macroUses.push_back(macroName);
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
                virtual auto OnMacroExpansion(const PPToken& macroUse) -> void override
                {
                    if (includeDepth == 0 && !macroUse.spelledRange.IsEmpty()) {
                        cache.macroUses.push_back(macroUse);
                    }
                }

            private:
                PreprocessInfoCache& cache;

                int includeDepth = 0;
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

    private:
        std::vector<PPToken> headerNames;
        std::vector<PPToken> macroUses;
    };

} // namespace glsld