#pragma once
#include "AstBase.h"
#include "LexContext.h"
#include "AstContext.h"
#include "SourceContext.h"
#include "PreprocessInfoCache.h"
#include "Uri.h"

#include "Compiler.h"

#include <mutex>
#include <condition_variable>
#include <filesystem>

namespace glsld
{
    enum class SymbolAccessType
    {
        Unknown,
        LayoutQualifier,
        Variable,
        Swizzle,
        MemberVariable,
        Parameter,
        Function,
        Type,
        InterfaceBlock,
    };

    struct SymbolAccessInfo
    {
        SyntaxToken token;
        DeclView symbolDecl;
        SymbolAccessType symbolType;
    };

    class LanguageQueryProvider
    {
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

        // Returns true if the specified token is in the main file
        auto InMainFile(const SyntaxToken& token) const -> bool
        {
            return GetLexContext().LookupSpelledFile(token) == GetSourceContext().GetMainFile()->GetID();
        }

        // Returns the text range of the specified AST node
        auto GetAstTextRange(const AstNodeBase& node) const -> TextRange
        {
            return GetLexContext().LookupExpandedTextRange(node.GetSyntaxRange());
        }

        // Returns the text range of the specified AST node, including the trailing whitespace and comments
        auto GetExtendedAstTextRange(const AstNodeBase& node) const -> TextRange
        {
            const auto& lexContext = GetLexContext();
            auto range             = node.GetSyntaxRange();

            TextPosition startPosition = lexContext.LookupSpelledTextRange(range.startTokenIndex).start;
            TextPosition endPosition   = lexContext.LookupSpelledTextRange(range.endTokenIndex).start;
            return TextRange{startPosition, endPosition};
        }

        // Returns the related information if the cursor position hits an identifier that's accessing a symbol
        auto LookupSymbolAccess(TextPosition position) const -> std::optional<SymbolAccessInfo>;

        // Returns true if the range of an AST node precedes the specified position
        auto PrecedesPosition(const AstNodeBase& node, TextPosition position) const -> bool
        {
            return GetLexContext().LookupExpandedTextRange(node.GetLastTokenIndex()).end <= position;
        }

        // Returns true if the range of an AST node succedes the specified position
        auto SucceedsPosition(const AstNodeBase& node, TextPosition position) const -> bool
        {
            return GetLexContext().LookupExpandedTextRange(node.GetFirstTokenIndex()).start > position;
        }

        // Returns true if the range of an AST node contains the specified position
        auto ContainsPosition(const AstNodeBase& node, TextPosition position) const -> bool
        {
            return GetAstTextRange(node).Contains(position);
        }

        // Returns true if the range of an AST node and trailing whitespaces contains the specified position
        auto ContainsPositionExtended(const AstNodeBase& node, TextPosition position) const -> bool
        {
            return GetExtendedAstTextRange(node).Contains(position);
        }

        // Returns true if the range of a syntax token and trailing whitespaces contains the specified position
        auto ContainsPositionExtended(const SyntaxToken& token, TextPosition position) const -> bool
        {
            auto tokenIndex     = token.index;
            auto nextTokenIndex = std::min<SyntaxTokenIndex>(tokenIndex + 1, GetLexContext().GetTokenCount() - 1);

            auto range = TextRange{GetLexContext().LookupSpelledTextRange(tokenIndex).start,
                                   GetLexContext().LookupSpelledTextRange(nextTokenIndex).start};
            return range.Contains(position);
        }

    private:
        friend class PendingIntellisenseProvider;

        CompilerObject compilerObject;
        PreprocessInfoCache ppInfoCache;
    };

    class PendingIntellisenseProvider
    {
    public:
        PendingIntellisenseProvider(int version, std::string uri, std::string sourceString)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString))
        {
        }

        auto Setup()
        {
            auto ppCallback = provider.ppInfoCache.GetCollectionCallback();

            provider.compilerObject.AddIncludePath(
                std::filesystem::path(Uri::FromString(uri)->GetPath().StdStrView()).parent_path());
            provider.compilerObject.Compile(sourceString, GetStandardLibraryModule(), ppCallback.get());

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