#include "Compiler.h"
#include "LanguageService.h"

#include <memory>

namespace glsld
{
    auto GetDefaultLibraryModule() -> std::shared_ptr<CompiledDependency>
    {
        static const auto defaultLibrarySource =
#include "DefaultLibrary.glsl.h"
            ;

        static const auto module = std::make_shared<CompiledDependency>(Compile(defaultLibrarySource));

        return module;
    }
} // namespace glsld
