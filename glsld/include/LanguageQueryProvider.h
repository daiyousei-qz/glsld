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

        InterfaceBlockType,

        InterfaceBlock,
    };

    struct SymbolAccessInfo
    {
        SyntaxToken token;
        DeclView symbolDecl;
        SymbolAccessType symbolType;
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

        // True if the specified token is in the main file.
        auto InMainFile(const SyntaxToken& token) const -> bool
        {
            return GetLexContext().LookupSpelledFile(token) == GetSourceContext().GetMainFile()->GetID();
        }

        // Returns the text range of the specified AST node.
        auto GetAstTextRange(const AstNode& node) const -> TextRange
        {
            return GetLexContext().LookupExpandedTextRange(node.GetSyntaxRange());
        }

        // Returns the text range of the specified AST node, including the trailing whitespace and comments.
        auto GetExtendedAstTextRange(const AstNode& node) const -> TextRange
        {
            const auto& lexContext = GetLexContext();
            auto range             = node.GetSyntaxRange();

            TextPosition startPosition = lexContext.LookupSpelledTextRange(range.startTokenIndex).start;
            TextPosition endPosition   = lexContext.LookupSpelledTextRange(range.endTokenIndex).start;
            return TextRange{startPosition, endPosition};
        }

        // Returns the text range of the specified syntax token, including the trailing whitespace and comments.
        auto GetExtendedTokenTextRange(SyntaxTokenIndex tokenIndex) const -> TextRange
        {
            auto nextTokenIndex = std::min<SyntaxTokenIndex>(tokenIndex + 1, GetLexContext().GetTokenCount() - 1);

            TextPosition startPosition = GetLexContext().LookupSpelledTextRange(tokenIndex).start;
            TextPosition endPosition   = GetLexContext().LookupSpelledTextRange(nextTokenIndex).start;
            return TextRange{startPosition, endPosition};
        }

        // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
        auto LookupSymbolAccess(TextPosition position) const -> std::optional<SymbolAccessInfo>;

        // True if the range of an AST node precedes the specified position.
        auto PrecedesPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetLexContext().LookupExpandedTextRange(node.GetLastTokenIndex()).end <= position;
        }

        // True if the range of an AST node succedes the specified position.
        auto SucceedsPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetLexContext().LookupExpandedTextRange(node.GetFirstTokenIndex()).start > position;
        }

        // True if the range of an AST node contains the specified position.
        auto ContainsPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetAstTextRange(node).Contains(position);
        }

        // Returns true if the range of an AST node and trailing whitespaces contains the specified position.
        auto ContainsPositionExtended(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExtendedAstTextRange(node).ContainsExtended(position);
        }

        auto ContainsPositionExtended(SyntaxTokenIndex tokenIndex, TextPosition position) const -> bool
        {
            return GetExtendedTokenTextRange(tokenIndex).ContainsExtended(position);
        }

        // Returns true if the range of a syntax token and trailing whitespaces contains the specified position.
        auto ContainsPositionExtended(const SyntaxToken& token, TextPosition position) const -> bool
        {
            return GetExtendedTokenTextRange(token.index).ContainsExtended(position);
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