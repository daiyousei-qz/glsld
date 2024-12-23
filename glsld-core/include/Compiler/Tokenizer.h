#pragma once
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Basic/StringView.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/SourceScanner.h"
#include "Compiler/Preprocessor.h"

namespace glsld
{
    // A tokenizer is responsible for lexing a source file and piping the tokens to preprocessor.
    // NOTE the tokenization process could be recursive and new instances of Tokenizer could be created when "#include"
    // is encountered.
    class Tokenizer final
    {
    private:
        const CompilerInvocationState& compiler;

        AtomTable& atomTable;

        PreprocessStateMachine& pp;

        FileID sourceFile;

        SourceScanner srcScanner;

        std::vector<char> tokenTextBuffer;

    public:
        // If `countUtf16Characters` is set to true, the tokenizer will count the number of UTF-16 code units in column
        // counter. Otherwise, the tokenizer will count the number of UTF-8 code units.
        Tokenizer(const CompilerInvocationState& compiler, PreprocessStateMachine& pp, AtomTable& atomTable,
                  FileID sourceFile, StringView sourceText)
            : compiler(compiler), atomTable(atomTable), pp(pp), sourceFile(sourceFile),
              srcScanner(sourceText, compiler.GetCompilerConfig().countUtf16Character)
        {
        }

        // Tokenize the source file and feed the tokens to the preprocessor.
        // This function should be called only once. After this function returns, this tokenizer object should no longer
        // be used.
        auto DoTokenize() -> void;

    private:
        // Lex the next PP token from the scanner or return an EOF token if the file is exhausted.
        auto LexPPToken() -> PPToken;

        // Assuming we are seeing "//", parse the line comment and return the token klass.
        auto ParseLineComment() -> TokenKlass;

        // Assuming we are seeing "/*", parse the block comment and return the token klass.
        auto ParseBlockComment() -> TokenKlass;

        // Assuming we are seeing `quoteStart`, parse the header name closed by `quoteEnd` and return the token klass.
        auto ParseHeaderName(char quoteStart, char quoteEnd, TokenKlass klass) -> TokenKlass;
    };

    auto TokenizeOnce(StringView text) -> std::tuple<TokenKlass, StringView, StringView>;
} // namespace glsld