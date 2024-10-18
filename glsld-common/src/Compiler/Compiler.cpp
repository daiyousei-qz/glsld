#include "Compiler/CompilerObject.h"
#include "Compiler/Preprocessor.h"
#include "Compiler/SourceContext.h"
#include "Compiler/DiagnosticStream.h"
#include "Compiler/LexContext.h"
#include "Compiler/AstContext.h"
#include "Compiler/Parser.h"

#include <atomic>

namespace glsld
{
    CompiledPreamble::CompiledPreamble()  = default;
    CompiledPreamble::~CompiledPreamble() = default;

    static auto GetNextCompileId() -> int
    {
        static std::atomic_int nextId = 0;
        return nextId++;
    }

    CompilerObject::CompilerObject()  = default;
    CompilerObject::~CompilerObject() = default;

    auto CompilerObject::CompilePreamble() -> std::shared_ptr<CompiledPreamble>
    {
        GLSLD_REQUIRE(!preamble);

        // Copy the preamble content since we'll have to export it to an object that outlives the compiler object.
        std::vector<char> systemPreambleContentCopy;
        std::vector<char> userPreambleContentCopy;
        systemPreambleContentCopy.assign(systemPreambleContent.begin(), systemPreambleContent.end());
        userPreambleContentCopy.assign(userPreambleContent.begin(), userPreambleContent.end());
        systemPreambleContent = StringView{systemPreambleContentCopy.data(), systemPreambleContentCopy.size()};
        userPreambleContent   = StringView{userPreambleContentCopy.data(), userPreambleContentCopy.size()};

        InitializeCompilation();
        if (!preamble) {
            DoPreprocess(FileID::SystemPreamble(), nullptr);
            DoPreprocess(FileID::UserPreamble(), nullptr);
        }
        DoParse();
        DoTypeCheck();
        FinalizeCompilation();

        auto result                   = std::make_shared<CompiledPreamble>();
        result->systemPreambleContent = std::move(systemPreambleContentCopy);
        result->userPreambleContent   = std::move(userPreambleContentCopy);
        result->sourceContext         = std::move(sourceContext);
        result->lexContext            = std::move(lexContext);
        result->astContext            = std::move(astContext);
        return std::move(result);
    }

    auto CompilerObject::CompileFromFile(StringView path, PPCallback* ppCallback) -> void
    {
        InitializeCompilation();
        if (!preamble) {
            DoPreprocess(FileID::SystemPreamble(), nullptr);
            DoPreprocess(FileID::UserPreamble(), nullptr);
        }
        DoPreprocess(sourceContext->OpenFromFile(path.StdStrView()), ppCallback);
        DoParse();
        DoTypeCheck();
        FinalizeCompilation();
    }

    auto CompilerObject::CompileFromBuffer(StringView sourceText, PPCallback* ppCallback) -> void
    {
        InitializeCompilation();
        if (!preamble) {
            DoPreprocess(FileID::SystemPreamble(), ppCallback);
            DoPreprocess(FileID::UserPreamble(), ppCallback);
        }
        DoPreprocess(sourceContext->OpenFromBuffer(sourceText), ppCallback);
        DoParse();
        DoTypeCheck();
        FinalizeCompilation();
    }

    auto CompilerObject::InitializeCompilation() -> void
    {
        // Initialize context for compilation
        this->sourceContext = std::make_unique<SourceContext>(preamble ? &preamble->GetSourceContext() : nullptr);
        if (!preamble) {
            this->sourceContext->SetSystemPreamble(systemPreambleContent);
            this->sourceContext->SetUserPreamble(userPreambleContent);
        }
        this->lexContext = std::make_unique<LexContext>(preamble ? &preamble->GetLexContext() : nullptr);
        this->astContext = std::make_unique<AstContext>(preamble ? &preamble->GetAstContext() : nullptr);
        this->diagStream = std::make_unique<DiagnosticStream>();
    }

    auto CompilerObject::DoPreprocess(FileID file, PPCallback* callback) -> void
    {
        GLSLD_REQUIRE(file.IsValid());

        GLSLD_TRACE_COMPILE_TIME("Lexing");
        Preprocessor{*this, callback, std::nullopt, 0}.PreprocessSourceFile(file);
    }
    auto CompilerObject::DoParse() -> void
    {
        GLSLD_TRACE_COMPILE_TIME("Parsing");
        Parser{*this}.DoParse();
    }
    auto CompilerObject::DoTypeCheck() -> void
    {
        GLSLD_TRACE_COMPILE_TIME("TypeChecking");
        // TODO: implement type checking
    }

    auto CompilerObject::FinalizeCompilation() -> void
    {
        if (config.dumpTokens) {
            for (int i = 0; i < lexContext->GetTUTokenCount(); ++i) {
                auto token        = lexContext->GetTUToken(lexContext->GetTUTokenIndexOffset() + i);
                auto expanedRange = lexContext->LookupExpandedTextRange(token.index);
                Print("[{}]'{}' @ ({},{}~{},{})\n", TokenKlassToString(token.klass), token.text.StrView(),
                      expanedRange.start.line, expanedRange.start.character, expanedRange.end.line,
                      expanedRange.end.character);
                if (token.klass == TokenKlass::Eof) {
                    auto eofFile = lexContext->LookupSpelledTextRange(token.index).fileID;
                    if (eofFile.IsSystemPreable()) {
                        Print("[System Preamble End]\n");
                    }
                    else if (eofFile.IsUserPreamble()) {
                        Print("[User Preamble End]\n");
                    }
                    else {
                        Print("[User File End]\n");
                    }
                }
            }
        }

        if (config.dumpAst) {
            GetAstContext().GetTranslationUnit()->Print();
        }

        sourceContext->Finalize();

        compiled = true;
    }

} // namespace glsld