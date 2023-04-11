#pragma once
#include "Common.h"
#include "SyntaxToken.h"
#include "SourceInfo.h"
#include "SourceScanner.h"
#include "Preprocessor.h"

namespace glsld
{
    // A tokenizer is responsible for lexing a source file and registering tokens into the LexContext.
    // In existance of preprocessor directives, the tokenizer will create PreprocessorHandler objects
    // to process them.
    // NOTE the tokenization process could be recursive and new instances of Tokenizer could be created when "#include"
    // is encountered.
    class Tokenizer final
    {
    public:
        Tokenizer(CompilerObject& compilerObject, Preprocessor& preprocessor, FileID sourceFile, StringView sourceText);

        // Tokenize the source file and register tokens into the LexContext.
        // This function should be called only once. After this function returns, this tokenizer object should no longer
        // be used.
        auto DoTokenize() -> void;

    private:
        // Lex the next PP token from the scanner.
        auto LexPPToken() -> PPToken;

        // Assuming we are seeing "//", parse the line comment and return the token klass.
        auto ParseLineComment() -> TokenKlass;

        // Assuming we are seeing "/*", parse the block comment and return the token klass.
        auto ParseBlockComment() -> TokenKlass;

        // Assuming we are seeing "\"", parse the quoted string and return the token klass.
        auto ParseQuotedString() -> TokenKlass;

        // Assuming we are seeing "<", parse the header name and return the token klass.
        auto ParseAngleString() -> TokenKlass;

        CompilerObject& compilerObject;

        Preprocessor& preprocessor;

        FileID sourceFile;

        SourceScanner srcScanner;

        std::vector<char> tokenTextBuffer;
    };
} // namespace glsld