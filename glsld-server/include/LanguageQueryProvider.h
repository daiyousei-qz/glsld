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
            return *compilerResult->GetUserFileAst();
        }

        auto GetPreprocessInfoCache() const -> const PreprocessInfoCache&
        {
            return ppInfoCache;
        }

        auto GetToken(SyntaxTokenID id) const -> const RawSyntaxTokenEntry*
        {
            ArrayView<RawSyntaxTokenEntry> tokens;
            switch (id.GetTU()) {
            case TranslationUnitID::SystemPreamble:
                tokens = compilerResult->GetSystemPreambleTokens();
                break;
            case TranslationUnitID::UserPreamble:
                tokens = compilerResult->GetUserPreambleTokens();
                break;
            case TranslationUnitID::UserFile:
                tokens = compilerResult->GetUserFileTokens();
                break;
            }

            return &tokens[id.GetTokenIndex()];
        }

        auto GetDotTokenIndex(const AstExpr& expr) const -> std::optional<SyntaxTokenID>
        {
            GLSLD_ASSERT(expr.Is<AstFieldAccessExpr>() || expr.Is<AstSwizzleAccessExpr>());
            auto range = expr.GetSyntaxRange();
            if (!range.Empty() && GetToken(range.GetBackID())->klass == TokenKlass::Dot) {
                // CASE 1: `a.b`
                return range.GetBackID();
            }
            else if (range.GetTokenCount() > 1 && GetToken(range.GetBackID() - 1)->klass == TokenKlass::Dot) {
                // CASE 2: `a. `
                return range.GetBackID() - 1;
            }
            else {
                return std::nullopt;
            }
        }

        auto FindTokenByTextPosition(TextPosition position) const -> const RawSyntaxTokenEntry*
        {
            auto tokens = compilerResult->GetUserFileTokens();
            auto it     = std::ranges::lower_bound(tokens, position, {},
                                                   [](const RawSyntaxTokenEntry& tok) { return tok.expandedRange.start; });
            if (it != tokens.end() && it != tokens.begin()) {
                return std::to_address(it);
            }
            else {
                return &tokens.back();
            }
        }

        // FIXME: we only support user file. is it good enough?
        auto FindPreceedingCommentTokens(SyntaxTokenID id) const -> ArrayView<RawCommentTokenEntry>
        {
            if (id.GetTU() != TranslationUnitID::UserFile) {
                return {};
            }

            auto [begin, end] =
                std::ranges::equal_range(compilerResult->GetUserFileComments(), id.GetTokenIndex(), {},
                                         [](const RawCommentTokenEntry& entry) { return entry.nextTokenIndex; });
            return {std::to_address(begin), std::to_address(end)};
        }

        auto LookupSpelledFile(SyntaxTokenID id) const -> FileID
        {
            return GetToken(id)->spelledFile;
        }

        // True if the specified token is spelled in the main file.
        auto IsSpelledInMainFile(SyntaxTokenID id) const -> bool
        {
            if (id.GetTU() != TranslationUnitID::UserFile) {
                return false;
            }

            return LookupSpelledFile(id) == compilerResult->GetMainFileID();
        }

        auto LookupSpelledTextRange(SyntaxTokenID id) const -> FileTextRange
        {
            auto token = GetToken(id);
            return FileTextRange{
                .fileID = token->spelledFile,
                .range  = token->spelledRange,
            };
        }

        auto LookupSpelledTextRangeInMainFile(SyntaxTokenID id) const -> std::optional<TextRange>
        {
            if (!IsSpelledInMainFile(id)) {
                return std::nullopt;
            }

            return GetToken(id)->spelledRange;
        }

        auto LookupExpandedTextRange(SyntaxTokenID id) const -> TextRange
        {
            return GetToken(id)->expandedRange;
        }

        auto GetExpandedTextRange(AstSyntaxRange range) const -> TextRange
        {
            if (range.Empty()) {
                auto firstTokenRange = LookupExpandedTextRange(range.GetBeginID());
                return TextRange{firstTokenRange.start, firstTokenRange.start};
            }
            else {
                TextPosition startPosition = LookupExpandedTextRange(range.GetBeginID()).start;
                TextPosition endPosition   = LookupExpandedTextRange(range.GetBackID()).end;
                return TextRange{startPosition, endPosition};
            }
        }

        // Returns the text range of the token being expanded to in the main file.
        auto GetExpandedTextRange(const SyntaxToken& token) const -> TextRange
        {
            return GetExpandedTextRange(token.GetSyntaxRange());
        }

        // Returns the text range of the AST node being expanded to in the main file.
        auto GetExpandedTextRange(const AstNode& node) const -> TextRange
        {
            return GetExpandedTextRange(node.GetSyntaxRange());
        }

        auto GetExpandedTextRangeExtended(AstSyntaxRange range) const -> TextRange
        {
            if (range.Empty()) {
                auto firstTokenRange = LookupExpandedTextRange(range.GetBeginID());
                return TextRange{firstTokenRange.start, firstTokenRange.start};
            }
            else {
                TextPosition startPosition = LookupExpandedTextRange(range.GetBeginID()).start;
                TextPosition endPosition   = LookupExpandedTextRange(range.GetEndID()).start;
                return TextRange{startPosition, endPosition};
            }
        }

        // Returns the text range of the token being expanded to in the main file, including the trailing whitespace and
        // comments.
        auto GetExpandedTextRangeExtended(const SyntaxToken& token) const -> TextRange
        {
            return GetExpandedTextRangeExtended(token.GetSyntaxRange());
        }

        // Returns the text range of the AST node being expanded to in the main file, including the trailing whitespace
        // and comments.
        auto GetExpandedTextRangeExtended(const AstNode& node) const -> TextRange
        {
            return GetExpandedTextRangeExtended(node.GetSyntaxRange());
        }

        // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
        auto LookupSymbolAccess(TextPosition position) const -> std::optional<SymbolAccessInfo>;

        // True if the expanded range of an AST node precedes the specified position in the main file.
        auto PrecedesPosition(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExpandedTextRange(node).end < position;
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

        auto ContainsPositionExtended(AstSyntaxRange range, TextPosition position) const -> bool
        {
            return GetExpandedTextRangeExtended(range).ContainsExtended(position);
        }

        // True if the expanded range of an AST node including trailing whitespaces contains the specified position in
        // the main file.
        auto ContainsPositionExtended(const AstNode& node, TextPosition position) const -> bool
        {
            return GetExpandedTextRangeExtended(node.GetSyntaxRange()).ContainsExtended(position);
        }

        // True if the expanded range of a token including trailing whitespaces contains the specified position in the
        // main file.
        auto ContainsPositionExtended(const SyntaxToken& token, TextPosition position) const -> bool
        {
            return GetExpandedTextRangeExtended(token.GetSyntaxRange()).ContainsExtended(position);
        }

        // True if the expanded range of a token including trailing whitespaces contains the specified position in the
        // main file.
        auto ContainsPositionExtended(SyntaxTokenID tokIndex, TextPosition position) const -> bool
        {
            // FIXME: implement properly
            return GetExpandedTextRangeExtended(SyntaxToken{tokIndex, TokenKlass::Invalid, {}})
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