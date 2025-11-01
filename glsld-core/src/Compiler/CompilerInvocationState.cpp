#include "Basic/AtomTable.h"
#include "Compiler/CompilerInvocationState.h"
#include "Compiler/AstContext.h"
#include "Compiler/CompilerArtifacts.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SymbolTable.h"
#include "Language/Stdlib.Generated.h"

#include <memory>

namespace glsld
{
    auto CompilerInvocationState::Initialize() -> void
    {
        if (preamble) {
            atomTable = std::make_unique<AtomTable>();
            atomTable->Import(preamble->GetAtomTable());
            // FIXME: import macros if preamble is present
            macroTable  = std::make_unique<MacroTable>();
            symbolTable = std::make_unique<SymbolTable>(preamble->GetSymbolTable().GetGlobalLevels());
            astContext  = std::make_unique<AstContext>(&preamble->GetAstContext());

            systemPreambleArtifacts = preamble->GetSystemPreambleArtifacts().CreateReference();
            userPreambleArtifacts   = preamble->GetUserPreambleArtifacts().CreateReference();
        }
        else {
            atomTable   = std::make_unique<AtomTable>();
            macroTable  = std::make_unique<MacroTable>();
            symbolTable = std::make_unique<SymbolTable>();
            astContext  = std::make_unique<AstContext>(nullptr);

            systemPreambleArtifacts = std::make_unique<CompilerArtifact>(TranslationUnitID::SystemPreamble);
            userPreambleArtifacts   = std::make_unique<CompilerArtifact>(TranslationUnitID::UserPreamble);
        }

        diagStream        = std::make_unique<DiagnosticStream>();
        userFileArtifacts = std::make_unique<CompilerArtifact>(TranslationUnitID::UserFile);
    }

    auto CompilerInvocationState::InitializeStdlib() -> void
    {
        // FIXME: don't run this when scanning for version and extensions
        GLSLD_ASSERT(preamble == nullptr);

        // Initialize system preamble
        if (!languageConfig.noStdlib) {
            sourceManager.SetSystemPreamble(GlslStdlibText);
        }

        // Initialize feature macros
        auto defineFeatureMacro = [this, one = atomTable->GetAtom("1")](StringView name) {
            macroTable->DefineFeatureMacro(atomTable->GetAtom(name), one);
        };

        defineFeatureMacro("GL_core_profile");

        switch (languageConfig.version) {
        default:
            // TODO: do we need support other version at all?
            macroTable->DefineFeatureMacro(atomTable->GetAtom("__GLSLD_LANGUAGE_VERSION"), atomTable->GetAtom("460"));
            break;
        }
        switch (languageConfig.profile) {
        default:
            // TODO: do we need support other profile at all?
            defineFeatureMacro("__GLSLD_LANGUAGE_PROFILE_CORE");
            break;
        }
        switch (languageConfig.stage) {
        case GlslShaderStage::Vertex:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_VERTEX");
            break;
        case GlslShaderStage::Fragment:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_FRAGMENT");
            break;
        case GlslShaderStage::Compute:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_COMPUTE");
            break;
        case GlslShaderStage::Geometry:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_GEOMETRY");
            break;
        case GlslShaderStage::TessControl:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_TESS_CTRL");
            break;
        case GlslShaderStage::TessEvaluation:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_TESS_EVAL");
            break;
        case GlslShaderStage::RayGeneration:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_RAY_GEN");
            break;
        case GlslShaderStage::RayIntersection:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_RAY_INTERSECT");
            break;
        case GlslShaderStage::RayAnyHit:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_RAY_ANY_HIT");
            break;
        case GlslShaderStage::RayClosestHit:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_RAY_CLOSEST_HIT");
            break;
        case GlslShaderStage::RayMiss:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_RAY_MISS");
            break;
        case GlslShaderStage::RayCallable:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_RAY_CALLABLE");
            break;
        case GlslShaderStage::Task:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_TASK");
            break;
        case GlslShaderStage::Mesh:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_MESH");
            break;
        default:
            defineFeatureMacro("__GLSLD_SHADER_STAGE_UNKNOWN");
            break;
        }

        // FIXME: define more feature macros based on extensions
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_INT8_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_INT16_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_INT32_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_INT64_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_FLOAT16_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_FLOAT32_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_FLOAT64_TYPE");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_SHADER_SM_BUILTIN_NV");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_MEMORY_SCOPE_SEMANTICS");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_RAY_QUERY");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_RAY_TRACING_EXT");
        defineFeatureMacro("__GLSLD_FEATURE_ENABLE_RAY_TRACING_NV");
    }

    auto CompilerInvocationState::TryDumpTokens(TranslationUnitID id, ArrayView<RawSyntaxToken> tokens) const -> void
    {
        if (compilerConfig.dumpTokens && id != TranslationUnitID::SystemPreamble) {
            if (id == TranslationUnitID::UserPreamble) {
                Print("=====Tokens of User Preamble=====\n");
            }
            else if (id == TranslationUnitID::UserFile) {
                Print("=====Tokens of User File=====\n");
            }

            for (const auto& token : tokens) {
                const auto& expanedRange = token.expandedRange;
                Print("[{}]'{}' @ ({},{}~{},{})\n", EnumToString(token.klass), token.text.StrView(),
                      expanedRange.start.line, expanedRange.start.character, expanedRange.end.line,
                      expanedRange.end.character);
            }
        }
    }

    auto CompilerInvocationState::TryDumpAst(TranslationUnitID id, const AstTranslationUnit* ast) const -> void
    {
        if (compilerConfig.dumpAst && id != TranslationUnitID::SystemPreamble) {
            if (id == TranslationUnitID::UserPreamble) {
                Print("=====AST of User Preamble=====\n");
            }
            else if (id == TranslationUnitID::UserFile) {
                Print("=====AST of User File=====\n");
            }

            Print("{}", ast->ToString());
        }
    }
} // namespace glsld