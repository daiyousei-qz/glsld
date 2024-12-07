#pragma once

#include "Language/ShaderTarget.h"
#include "Language/Extension.h"

#include <filesystem>
#include <vector>
#include <string>

namespace glsld
{
    enum class CompileMode
    {
        ParseOnly,
        PreprocessOnly,
    };

    struct CompilerConfig
    {
        // Dumps the token stream in stdout.
        bool dumpTokens = false;

        // Dumps the parsed AST in stdout.
        bool dumpAst = false;

        // Whether the compiler should skip tokens in the preamble.
        // The user preamble is defined as all tokens before any non-comment valid tokens in the main file.
        // bool skipUserPreamble = false;

        // While we only support source file encoded in utf-8, we may need count characters in utf-16 code points
        // to satisfy the language server protocol spec.
        bool countUtf16Character = false;

        CompileMode mode = CompileMode::ParseOnly;

        // The maximum number of nested include levels.
        int maxIncludeDepth = 32;

        // The include paths to search for included files.
        std::vector<std::filesystem::path> includePaths;

        // The preprocessor definitions.
        std::vector<std::string> defines;
    };

    struct LanguageConfig
    {
        GlslVersion version        = GlslVersion::Ver460;
        GlslProfile profile        = GlslProfile::Core;
        GlslShaderStage stage      = GlslShaderStage::Unknown;
        ExtensionStatus extensions = {};
        bool noStdlib              = false;
    };
} // namespace glsld