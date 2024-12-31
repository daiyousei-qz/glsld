#pragma once
#include "Ast/Expr.h"
#include "Ast/Misc.h"
#include "Basic/Common.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/CompilerResult.h"
#include "Compiler/SyntaxToken.h"
#include "PreprocessInfoCache.h"
#include "Uri.h"

#include <mutex>
#include <condition_variable>

namespace glsld
{
    inline auto GetStdlibModule() -> const std::shared_ptr<PrecompiledPreamble>&
    {
        static std::shared_ptr<PrecompiledPreamble> stdlibPreamble = []() {
            CompilerInvocation compiler;
            return compiler.CompilePreamble(nullptr);
        }();

        return stdlibPreamble;
    }

    enum class SymbolAccessType
    {
        // The symbol is an identifier with none of the following categoty.
        Unknown,

        // The symbol is a key in the layout qualifier. e.g. `location` in `layout(location = 0) in vec4 a;`
        LayoutQualifier,

        // The symbol is a global variable name. either in a variable declaration or accessing of a variable.
        GlobalVariable,

        // The symbol is a local variable name. either in a variable declaration or accessing of a variable.
        LocalVariable,

        // The symbol is a swizzle name. e.g. `xyz` in `vec3 v; v.xyz;`
        Swizzle,

        // The symbol is accessing of a struct member. e.g. `x` in `s.x;`
        MemberVariable,

        // The symbol is a function parameter name.
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
        AstSyntaxToken token;

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
        friend class PendingBackgroundCompilation;

        std::unique_ptr<CompilerResult> compilerResult = nullptr;
        PreprocessInfoCache ppInfoCache;

    public:
        LanguageQueryProvider(std::unique_ptr<CompilerResult> result, PreprocessInfoCache ppInfo)
            : compilerResult(std::move(result)), ppInfoCache(std::move(ppInfo))
        {
        }

        auto GetCompilerResult() const -> const CompilerResult&
        {
            return *compilerResult;
        }

        auto GetUserFileAst() const -> const AstTranslationUnit&
        {
            return *compilerResult->GetUserFileArtifacts().GetAst();
        }

        auto GetPreprocessInfoCache() const -> const PreprocessInfoCache&
        {
            return ppInfoCache;
        }

        // Returns the token entry of the specified token.
        auto LookupToken(SyntaxTokenID id) const -> const RawSyntaxToken*;

        // Returns the token entries of the specified token range.
        auto LookupTokens(AstSyntaxRange range) const -> ArrayView<RawSyntaxToken>;

        auto LookupDotTokenIndex(const AstExpr& expr) const -> std::optional<SyntaxTokenID>;

        // Returns the token entries that is expanded to the specified position.
        auto LookupTokenByPosition(TextPosition position) const -> ArrayView<RawSyntaxToken>;

        // Returns the token entries that is expanded to the specified line.
        auto LookupTokenByLine(uint32_t lineNum) const -> ArrayView<RawSyntaxToken>;

        auto LookupPreceedingComment(SyntaxTokenID id) const -> ArrayView<RawCommentToken>;

        auto LookupSpelledFile(SyntaxTokenID id) const -> FileID;

        // True if the specified token is spelled in the main file.
        auto IsSpelledInMainFile(SyntaxTokenID id) const -> bool;

        // Returns the spelled text range of the specified token.
        auto LookupSpelledTextRange(SyntaxTokenID id) const -> FileTextRange;

        // Returns the spelled text range of the specified token in the main file.
        // If the token is not spelled in the main file, returns nullopt.
        auto LookupSpelledTextRangeInMainFile(SyntaxTokenID id) const -> std::optional<TextRange>;

        // Returns the expanded text range of the specified token.
        // Notably, expanded text range is always in the main file.
        auto LookupExpandedTextRange(SyntaxTokenID id) const -> TextRange;

        // Returns the expanded text range of the specified token range.
        // Notably, expanded text range is always in the main file.
        auto LookupExpandedTextRange(AstSyntaxRange range) const -> TextRange;

        auto LookupExpandedTextRange(const AstSyntaxToken& token) const -> TextRange
        {
            return LookupExpandedTextRange(token.id);
        }

        auto LookupExpandedTextRange(const AstNode& node) const -> TextRange
        {
            return LookupExpandedTextRange(node.GetSyntaxRange());
        }

        // Returns the expanded text range of the specific token range, including the trailing whitespace and comments.
        // Notably, expanded text range is always in the main file.
        auto LookupExpandedTextRangeExtended(AstSyntaxRange range) const -> TextRange;

        auto LookupExpandedTextRangeExtended(const AstSyntaxToken& token) const -> TextRange
        {
            return LookupExpandedTextRangeExtended(token.GetSyntaxRange());
        }

        auto LookupExpandedTextRangeExtended(const AstNode& node) const -> TextRange
        {
            return LookupExpandedTextRangeExtended(node.GetSyntaxRange());
        }

        // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
        auto QuerySymbolByPosition(TextPosition position) const -> std::optional<SymbolAccessInfo>;

        // True if the expanded range of an AST node precedes the specified position in the main file.
        auto PrecedesPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return LookupExpandedTextRange(node).end < position;
        }

        // True if the expanded range of an AST node succedes the specified position in the main file.
        auto SucceedsPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return LookupExpandedTextRange(node).start > position;
        }

        // True if the expanded range of an AST node contains the specified position in the main file.
        auto ContainsPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return LookupExpandedTextRange(node).Contains(position);
        }

        auto ContainsPosition(const AstSyntaxToken& token, TextPosition position) const -> bool
        {
            return LookupExpandedTextRange(token).Contains(position);
        }

        auto ContainsPositionExtended(AstSyntaxRange range, TextPosition position) const -> bool
        {
            return LookupExpandedTextRangeExtended(range).ContainsExtended(position);
        }

        // True if the expanded range of an AST node including trailing whitespaces contains the specified position in
        // the main file.
        auto ContainsPositionExtended(const AstNode& node, TextPosition position) const -> bool
        {
            return LookupExpandedTextRangeExtended(node.GetSyntaxRange()).ContainsExtended(position);
        }

        // True if the expanded range of a token including trailing whitespaces contains the specified position in the
        // main file.
        auto ContainsPositionExtended(const AstSyntaxToken& token, TextPosition position) const -> bool
        {
            return LookupExpandedTextRangeExtended(token.GetSyntaxRange()).ContainsExtended(position);
        }

        // True if the expanded range of a token including trailing whitespaces contains the specified position in the
        // main file.
        auto ContainsPositionExtended(SyntaxTokenID tokIndex, TextPosition position) const -> bool
        {
            // FIXME: implement properly
            return LookupExpandedTextRangeExtended(AstSyntaxToken{tokIndex, TokenKlass::Invalid, {}})
                .ContainsExtended(position);
        }
    };

    class PendingBackgroundCompilation
    {
    private:
        // Document version
        int version;
        std::string uri;
        std::string sourceString;

        std::unique_ptr<CompilerInvocation> compiler    = nullptr;
        std::unique_ptr<LanguageQueryProvider> provider = nullptr;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;

    public:
        PendingBackgroundCompilation(int version, std::string uri, std::string sourceString)
            : version(version), uri(std::move(uri)), sourceString(std::move(sourceString))
        {
        }

        auto Setup()
        {
            PreprocessInfoCache ppInfoCache;
            auto ppCallback = ppInfoCache.GetCollectionCallback();

            compiler = std::make_unique<CompilerInvocation>(GetStdlibModule());
            compiler->SetCountUtf16Characters(true);
            compiler->AddIncludePath(std::filesystem::path(Uri::FromString(uri)->GetPath().StdStrView()).parent_path());
            compiler->SetMainFileFromBuffer(sourceString);
            auto result = compiler->CompileMainFile(ppCallback.get());

            provider = std::make_unique<LanguageQueryProvider>(std::move(result), std::move(ppInfoCache));

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
            return *provider;
        }
    };
} // namespace glsld