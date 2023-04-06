#include "Compiler.h"
#include "LanguageService.h"

#include <memory>

namespace glsld
{
    static auto CreateDefaultLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static constexpr StringView defaultLibrarySource =
#include "DefaultLibrary.glsl.h"
            ;

        CompilerObject compilerObject{};
        compilerObject.Compile(defaultLibrarySource);
        return compilerObject.CreatePreamble();
    }

    auto GetDefaultLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static const std::shared_ptr<CompiledPreamble> module = CreateDefaultLibraryModule();

        return module;
    }
} // namespace glsld
