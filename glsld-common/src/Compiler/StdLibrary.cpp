#include "Compiler/CompilerObject.h"

namespace glsld
{
    static auto CreateStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static constexpr StringView standardLibrarySource =
#include "Language/DefaultLibrary.glsl.h"
            ;

        CompilerObject compilerObject{};
        compilerObject.SetSystemPreamble(standardLibrarySource);
        return compilerObject.CompilePreamble();
    }

    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static const std::shared_ptr<CompiledPreamble> module = CreateStandardLibraryModule();
        return module;
    }
} // namespace glsld