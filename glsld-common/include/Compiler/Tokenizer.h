#pragma once
#include "Basic/Common.h"
#include "Basic/SourceInfo.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/SourceScanner.h"
#include "Compiler/Preprocessor.h"

namespace glsld
{
    // A tokenizer is responsible for lexing a source file and piping the tokens to preprocessor.
    // In existance of preprocessor directives, the tokenizer will create PreprocessorHandler objects
    // to process them.
    // NOTE the tokenization process could be recursive and new instances of Tokenizer could be created when "#include"
    // is encountered.
    class Tokenizer final
    {
    private:
        CompilerObject& compilerObject;

        Preprocessor& preprocessor;

        FileID sourceFileId;

        SourceScanner srcScanner;

        std::vector<char> tokenTextBuffer;

    public:
        // If `countUtf16Characters` is set to true, the tokenizer will count the number of UTF-16 code units in column
        // counter. Otherwise, the tokenizer will count the number of UTF-8 code units.
        Tokenizer(CompilerObject& compilerObject, Preprocessor& preprocessor, FileID sourceFile, StringView sourceText,
                  bool countUtf16Characters = true);

        // Tokenize the source file and register tokens into the LexContext.
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

        // Assuming we are seeing "\"", parse the quoted string and return the token klass.
        auto ParseQuotedString() -> TokenKlass;

        // Assuming we are seeing "<", parse the header name and return the token klass.
        auto ParseAngleString() -> TokenKlass;
    };
} // namespace glsld