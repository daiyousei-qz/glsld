#include "Compiler/CompilerObject.h"

namespace glsld
{
    static auto CreateStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static constexpr StringView standardLibrarySource =
#include "Language/DefaultLibrary.glsl.h"
            ;

        CompilerObject compilerObject{};
        compilerObject.CompileFromBuffer(standardLibrarySource, nullptr, nullptr);
        return compilerObject.CreatePreamble();
    }

    auto GetStandardLibraryModule() -> std::shared_ptr<CompiledPreamble>
    {
        static const std::shared_ptr<CompiledPreamble> module = CreateStandardLibraryModule();
        return module;
    }
} // namespace glsld