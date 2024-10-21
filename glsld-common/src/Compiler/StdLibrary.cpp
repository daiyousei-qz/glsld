#include "Compiler/CompilerObject.h"

namespace glsld
{
    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static const std::shared_ptr<CompiledPreamble> module = [] {
            CompilerObject compilerObject{};
            return compilerObject.CompilePreamble();
        }();

        return module;
    }
} // namespace glsld