#include "Compiler.h"
#include "LanguageService.h"

#include <memory>

namespace glsld
{
    auto GetDefaultLibraryModule() -> std::shared_ptr<CompiledExternalModule>
    {
        static const auto defaultLibrarySource =
#include "DefaultLibrary.glsl.h"
            ;

        static const auto module = GlslCompiler{}.CompileExternalModule(defaultLibrarySource);

        return module;
    }
} // namespace glsld
