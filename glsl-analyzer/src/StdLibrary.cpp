#include "Compiler.h"

namespace glsld
{
    static auto CreateStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static constexpr StringView standardLibrarySource =
#include "DefaultLibrary.glsl.h"
            ;

        CompilerObject compilerObject{};
        compilerObject.Compile(standardLibrarySource, nullptr, nullptr);
        return compilerObject.CreatePreamble();
    }

    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static const std::shared_ptr<CompiledPreamble> module = CreateStandardLibraryModule();
        return module;
    }
} // namespace glsld