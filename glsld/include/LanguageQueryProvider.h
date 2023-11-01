#pragma once
#include "Basic/Common.h"
#include "Compiler/LexContext.h"
#include "Compiler/AstContext.h"
#include "Compiler/SourceContext.h"
#include "Compiler/CompilerObject.h"
#include "PreprocessInfoCache.h"

#include "Uri.h"

#include <mutex>
#include <condition_variable>

namespace glsld
{
    enum class SymbolAccessType
    {
        // The symbol is an identifier with non of the following categoty.
        Unknown,

        // The symbol is a key in the layout qualifier. e.g. `location` in `layout(location = 0) in vec4 a;`
        LayoutQualifier,

        // The symbol is variable name. either in a variable declaration or accessing of a variable.
        Variable,

        // The symbol is a swizzle name. e.g. `xyz` in `vec3 v; v.xyz;`
        Swizzle,

        // The symbol is accessing of a struct member. e.g. `x` in `s.x;`
        MemberVariable,

        // The symbol is accessing of a function parameter.
        Parameter,

        // The symbol is a function name, either in a function call or a function declaration.
        Function,

        // The symbol is a type name, either in a type declaration or a type specifier.
        Type,

        // The symbol is a block name. e.g. `Block` in `uniform Block { ... }`
        InterfaceBlock,

        // The symbol is a block instance name. e.g. `block` in `uniform Block { ... } block;`
        InterfaceBlockInstance,
    };

    struct SymbolAccessInfo
    {
        // The token that the cursor is on.
        SyntaxToken token;

        // The declaration of the symbol that the token is accessing.
        DeclView symbolDecl;

        // The type of the symbol that the token is accessing.
        SymbolAccessType symbolType;

        // True if the token is from the declaration. e.g. `a` in `int a;`
        bool isDeclName = false;
    };

    class LanguageQueryProvider
    {
    private:
        friend class PendingLanguageQueryProvider;

        CompilerObject compilerObject;
        PreprocessInfoCache ppInfoCache;

    public:
        auto GetCompilerObject() const -> const CompilerObject&
        {
            return compilerObject;
        }

        auto GetLexContext() const -> const LexContext&
        {
            return compilerObject.GetLexContext();
        }

        auto GetAstContext() const -> const AstContext&
        {
            return compilerObject.GetAstContext();
        }

        auto GetSourceContext() const -> const SourceContext&
        {
            return compilerObject.GetSourceContext();
        }

        auto GetPreprocessInfoCache() const -> const PreprocessInfoCache&
        {
            return ppInfoCache;
        }

        // True if the specified token is spelled in the main file.
        auto IsSpelledInMainFile(const SyntaxToken& token) const -> bool
        {
            return GetLexContext().LookupSpelledFile(token.index) == GetLexContext().GetTUMainFileID();
        }

        auto GetSpelledTextRange(const SyntaxToken& token) const -> FileTextRange
        {
            return GetLexContext().LookupSpelledTextRange(token.index);
        }

        auto GetSpelledTextRangeInMainFile(const SyntaxToken& token) const -> std::optional<TextRange>
        {
            if (!IsSpelledInMainFile(token)) {
                return std::nullopt;
            }

            return GetLexContext().LookupSpelledTextRange(token.index).range;
        }

        // Returns the text range of the token being expanded to in the main file.
        auto GetExpandedTextRange(const SyntaxToken& token) const -> TextRange
        {
            return GetLexContext().LookupExpandedTextRange(token.index);
        }

        // Returns the text range of the AST node being expanded to in the main file.
        auto GetExpandedTextRange(const AstNode& node) const -> TextRange
        {
            auto range = node.GetSyntaxRange();
            GLSLD_ASSERT(range.endTokenIndex >= range.startTokenIndex);

            if (range.endTokenIndex == range.startTokenIndex) {
                auto firstTokenRange = GetLexContext().LookupExpandedTextRange(range.startTokenIndex);
                return TextRange{firstTokenRange.start, firstTokenRange.start};
            }
            else {
                TextPosition startPosition = GetLexContext().LookupExpandedTextRange(range.startTokenIndex).start;
                TextPosition endPosition   = GetLexContext().LookupExpandedTextRange(range.endTokenIndex - 1).end;
                return TextRange{startPosition, endPosition};
            }
        }

        // Returns the text range of the token being expanded to in the main file, including the trailing whitespace and
        // comments.
        auto GetExpandedTextRangeExtended(const SyntaxToken& token) const -> TextRange
        {
            auto nextTokenIndex = std::min<SyntaxTokenIndex>(token.index + 1, GetLexContext().GetTotalTokenCount() - 1);

            TextPosition startPosition = GetLexContext().LookupExpandedTextRange(token.index).start;
            TextPosition endPosition   = GetLexContext().LookupExpandedTextRange(nextTokenIndex).start;
            return TextRange{startPosition, endPosition};
        }

        // Returns the text range of the AST node being expanded to in the main file, including the trailing whitespace
        // and comments.
        auto GetExpandedTextRangeExtended(const AstNode& node) const -> TextRange
        {
            auto range = node.GetSyntaxRange();
            GLSLD_ASSERT(range.endTokenIndex >= range.startTokenIndex);

            if (range.endTokenIndex == range.startTokenIndex) {
                auto firstTokenRange = GetLexContext().LookupExpandedTextRange(range.startTokenIndex);
                return TextRange{firstTokenRange.start, firstTokenRange.start};
            }
            else {
                TextPosition startPosition = GetLexContext().LookupExpandedTextRange(range.startTokenIndex).start;
                TextPosition endPosition   = GetLexContext().LookupExpandedTextRange(range.endTokenIndex).start;
                return TextRange{startPosition, endPosition};
            }
        }

        // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
        auto LookupSymbolAccess(TextPosition position) const -> std::optional<SymbolAccessInfo>;

        // True if the expanded range of an AST node precedes the specified position in the main file.
        auto PrecedesPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExpandedTextRange(node).end <= position;
        }

        // True if the expanded range of an AST node succedes the specified position in the main file.
        auto SucceedsPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExpandedTextRange(node).start > position;
        }

        // True if the expanded range of an AST node contains the specified position in the main file.
        auto ContainsPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExpandedTextRange(node).Contains(position);
        }

        auto ContainsPosition(const SyntaxToken& token, TextPosition position) const -> bool
        {
            return GetExpandedTextRange(token).Contains(position);
        }

        // True if the expanded range of an AST node including trailing whitespaces contains the specified position in
        // the main file.
        auto ContainsPositionExtended(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExpandedTextRangeExtended(node).ContainsExtended(position);
        }

        // True if the expanded range of a token including trailing whitespaces contains the specified position in the
        // main file.
        auto ContainsPositionExtended(const SyntaxToken& token, TextPosition position) const -> bool
        {
            return GetExpandedTextRangeExtended(token).ContainsExtended(position);
        }

        // True if the expanded range of a token including trailing whitespaces contains the specified position in the
        // main file.
        auto ContainsPositionExtended(SyntaxTokenIndex tokIndex, TextPosition position) const -> bool
        {
            // FIXME: implement properly
            return GetExpandedTextRangeExtended(SyntaxToken{tokIndex, TokenKlass::Invalid, {}})
                .ContainsExtended(position);
        }
    };

    class PendingLanguageQueryProvider
    {
    public:
        PendingLanguageQueryProvider(int version, std::string uri, std::string sourceString)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString))
        {
        }

        auto Setup()
        {
            auto ppCallback = provider.ppInfoCache.GetCollectionCallback();

            provider.compilerObject.AddIncludePath(
                std::filesystem::path(Uri::FromString(uri)->GetPath().StdStrView()).parent_path());
            provider.compilerObject.CompileFromBuffer(sourceString, GetStandardLibraryModule(), ppCallback.get());

            std::unique_lock<std::mutex> lock{mu};
            available = true;
            cv.notify_all();
        }

        auto WaitAvailable() -> bool
        {
            using namespace std::literals;
            std::unique_lock<std::mutex> lock{mu};
            if (available || cv.wait_for(lock, 1s) == std::cv_status::no_timeout) {
                return available;
            }

            return false;
        }

        auto StealBuffer() -> std::string
        {
            std::unique_lock<std::mutex> lock{mu};
            if (available) {
                // After compilation finishes, the sourceString buffer is no longer needed
                return std::move(sourceString);
            }
            else {
                return sourceString;
            }
        }

        auto GetProvider() -> const LanguageQueryProvider&
        {
            GLSLD_ASSERT(available);
            return provider;
        }

    private:
        int version;
        std::string uri;
        std::string sourceString;

        LanguageQueryProvider provider;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;
    };
} // namespace glsld