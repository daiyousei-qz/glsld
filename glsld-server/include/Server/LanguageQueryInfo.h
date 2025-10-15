#pragma once
#include "Ast/Expr.h"
#include "Ast/Misc.h"
#include "Basic/Common.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/CompilerResult.h"
#include "Compiler/SyntaxToken.h"
#include "Server/PreprocessSymbolStore.h"

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

    enum class SymbolDeclType
    {
        // The symbol is a header name. e.g. `"header.h"` in `#include "header.h"`
        HeaderName,

        // The symbol is a macro name. e.g. `MACRO` in `#define MACRO 1`
        Macro,

        // The symbol is a key in the layout qualifier. e.g. `location` in `layout(location = 0) in vec4 a;`
        LayoutQualifier,

        // The symbol is a global variable name. either in a variable declaration or accessing of a variable.
        GlobalVariable,

        // The symbol is a local variable name. either in a variable declaration or accessing of a variable.
        LocalVariable,

        // The symbol is a swizzle name. e.g. `xyz` in `vec3 v; v.xyz;`
        Swizzle,

        // The symbol is accessing of a struct member. e.g. `x` in `s.x;`
        StructMember,

        // The symbol is a function parameter name.
        Parameter,

        // The symbol is a function name, either in a function call or a function declaration.
        Function,

        // The symbol is a type name, either in a type declaration or a type specifier.
        Type,

        // The symbol is a block name. e.g. `Block` in `uniform Block { ... }`
        Block,

        // The symbol is a block instance name. e.g. `block` in `uniform Block { ... } block;`
        BlockInstance,

        // The symbol is a block member name. e.g. `member` in `uniform Block { int member; }`
        BlockMember,
    };

    struct SymbolQueryResult
    {
        // The type of the symbol that the token is accessing.
        SymbolDeclType symbolType;

        // Usually, this is the range of the identifier token.
        // But in case we hit a macro expansion, this is the range of the expaned tokens.
        AstSyntaxRange symbolRange;

        // The text of the symbol spelled in the source file.
        std::string spelledText;

        // The range of the spelled text.
        TextRange spelledRange;

        // The absolute path of the header file if the symbol is a header name.
        const PPSymbolOccurrence* ppSymbolOccurrence = nullptr;

        // The smallest AST node that possesses the symbol.
        const AstNode* astSymbolOccurrence = nullptr;

        // The declaration AST node of the symbol, if any.
        const AstDecl* symbolDecl = nullptr;

        // True if the token is from the declaration. e.g. `a` in `int a;`
        bool isDeclaration = false;
    };

    class LanguageQueryInfo
    {
    private:
        friend class BackgroundCompilation;

        std::unique_ptr<CompilerResult> compilerResult     = nullptr;
        std::unique_ptr<PreprocessSymbolStore> ppInfoStore = nullptr;

    public:
        LanguageQueryInfo(std::unique_ptr<CompilerResult> result, std::unique_ptr<PreprocessSymbolStore> ppInfoStore)
            : compilerResult(std::move(result)), ppInfoStore(std::move(ppInfoStore))
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

        auto GetPreprocessInfo() const -> const PreprocessSymbolStore&
        {
            return *ppInfoStore;
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

        // True if the specified file is the main file.
        auto IsMainFile(FileID file) const -> bool;

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

        // TODO: Should we support query for non-identifier?
        // Returns the related information if the cursor position hits an identifier that's accessing a symbol.
        auto QuerySymbolByPosition(TextPosition position) const -> std::optional<SymbolQueryResult>;
    };
} // namespace glsld