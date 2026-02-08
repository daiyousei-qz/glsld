#pragma once

#include "Support/StringView.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>

namespace glsld::lsp
{
    // /**
    //  * Defines an integer number in the range of -2^31 to 2^31 - 1.
    //  */
    using integer = int32_t;

    // /**
    //  * Defines an unsigned integer number in the range of 0 to 2^31 - 1.
    //  */
    using uinteger = uint32_t;

    // /**
    //  * Defines a decimal number. Since decimal numbers are very
    //  * rare in the language server specification we denote the
    //  * exact range with every decimal using the mathematics
    //  * interval notation (e.g. [0, 1] denotes all decimals d with
    //  * 0 <= d <= 1.
    //  */
    using decimal = float;

    using Uri         = std::string;
    using DocumentUri = Uri;

    struct Position
    {
        // /**
        //  * Line position in a document (zero-based).
        //  */
        // line: uinteger;
        uinteger line;

        // 	/**
        //  * Character offset on a line in a document (zero-based). The meaning of this
        //  * offset is determined by the negotiated `PositionEncodingKind`.
        //  *
        //  * If the character value is greater than the line length it defaults back
        //  * to the line length.
        //  */
        // character: uinteger;
        uinteger character;
    };

    struct Range
    {
        // /**
        //  * The range's start position.
        //  */
        // start: Position;
        Position start;

        // /**
        //  * The range's end position.
        //  */
        // end: Position;
        Position end;
    };

    struct TextDocumentItem
    {
        // /**
        //  * The text document's URI.
        //  */
        //  uri: DocumentUri;
        DocumentUri uri;

        //  /**
        //   * The text document's language identifier.
        //   */
        //  languageId: string;
        std::string languageId;

        //  /**
        //   * The version number of this document (it will increase after each
        //   * change, including undo/redo).
        //   */
        //  version: integer;
        integer version;

        //  /**
        //   * The content of the opened text document.
        //   */
        //  text: string;
        std::string text;
    };

    struct TextDocumentIdentifier
    {
        // /**
        //  * The text document's URI.
        //  */
        // uri: DocumentUri;
        DocumentUri uri;
    };

    struct VersionedTextDocumentIdentifier
    {
        // /**
        //  * The text document's URI.
        //  */
        // uri: DocumentUri;
        DocumentUri uri;

        // /**
        //  * The version number of this document.
        //  *
        //  * The version number of a document will increase after each change,
        //  * including undo/redo. The number doesn't need to be consecutive.
        //  */
        //  version: integer;
        integer version;
    };

    struct OptionalVersionedTextDocumentIdentifier
    {
        // /**
        //  * The text document's URI.
        //  */
        // uri: DocumentUri;
        DocumentUri uri;

        // /**
        //  * The version number of this document. If an optional versioned text document
        //  * identifier is sent from the server to the client and the file is not
        //  * open in the editor (the server has not received an open notification
        //  * before) the server can send `null` to indicate that the version is
        //  * known and the content on disk is the master (as specified with document
        //  * content ownership).
        //  *
        //  * The version number of a document will increase after each change,
        //  * including undo/redo. The number doesn't need to be consecutive.
        //  */
        //  version: integer | null;
        std::optional<integer> version;
    };

    struct TextDocumentPositionParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;
    };

    struct TextEdit
    {
        // /**
        //  * The range of the text document to be manipulated. To insert
        //  * text into a document create a range where start === end.
        //  */
        // range: Range;
        Range range;

        // /**
        //  * The string to be inserted. For delete operations use an
        //  * empty string.
        //  */
        // newText: string;
        std::string newText;
    };

    struct Location
    {
        // uri: DocumentUri;
        DocumentUri uri;

        // range: Range;
        Range range;
    };

    // /**
    //  * Describes the content type that a client supports in various
    //  * result literals like `Hover`, `ParameterInfo` or `CompletionItem`.
    //  *
    //  * Please note that `MarkupKinds` must not start with a `$`. This kinds
    //  * are reserved for internal usage.
    //  */
    //  export namespace MarkupKind {
    // 	/**
    // 	 * Plain text is supported as a content format
    // 	 */
    // 	export const PlainText: 'plaintext' = 'plaintext';

    // 	/**
    // 	 * Markdown is supported as a content format
    // 	 */
    // 	export const Markdown: 'markdown' = 'markdown';
    // }
    // export type MarkupKind = 'plaintext' | 'markdown';

    // /**
    //  * A `MarkupContent` literal represents a string value which content is
    //  * interpreted base on its kind flag. Currently the protocol supports
    //  * `plaintext` and `markdown` as markup kinds.
    //  *
    //  * If the kind is `markdown` then the value can contain fenced code blocks like
    //  * in GitHub issues.
    //  *
    //  * Here is an example how such a string can be constructed using
    //  * JavaScript / TypeScript:
    //  * ```typescript
    //  * let markdown: MarkdownContent = {
    //  * 	kind: MarkupKind.Markdown,
    //  * 	value: [
    //  * 		'# Header',
    //  * 		'Some text',
    //  * 		'```typescript',
    //  * 		'someCode();',
    //  * 		'```'
    //  * 	].join('\n')
    //  * };
    //  * ```
    //  *
    //  * *Please Note* that clients might sanitize the return markdown. A client could
    //  * decide to remove HTML from the markdown to avoid script execution.
    //  */
    class MarkupContent
    {
    private:
        // /**
        //  * The type of the Markup
        //  */
        // kind: MarkupKind;
        bool isMarkdown = false;

        // /**
        //  * The content of the message.
        //  */
        // value: string;
        std::string value = "";

    public:
        MarkupContent() = default;
        MarkupContent(bool markdown, std::string value) : isMarkdown(markdown), value(std::move(value))
        {
        }

        auto GetValue() const -> StringView
        {
            return value;
        }

        auto Serialize() const -> nlohmann::json
        {
            return nlohmann::json::object({{"kind", isMarkdown ? "markdown" : "plaintext"}, {"value", value}});
        }

        auto Deserialize(const nlohmann::json& j) -> bool
        {
            if (!j.is_object()) {
                return false;
            }

            if (auto it = j.find("kind"); it != j.end()) {
                if (!it->is_string()) {
                    return false;
                }

                isMarkdown = it->get<std::string>() == "markdown";
            }

            if (auto it = j.find("value"); it != j.end()) {
                if (!it->is_string()) {
                    return false;
                }

                value = it->get<std::string>();
            }

            return true;
        }
    };

    struct WorkspaceEdit
    {
        // /**
        //  * Holds changes to existing resources.
        //  */
        //  changes?: { [uri: DocumentUri]: TextEdit[]; };

        //  /**
        //   * Depending on the client capability
        //   * `workspace.workspaceEdit.resourceOperations` document changes are either
        //   * an array of `TextDocumentEdit`s to express changes to n different text
        //   * documents where each text document edit addresses a specific version of
        //   * a text document. Or it can contain above `TextDocumentEdit`s mixed with
        //   * create, rename and delete file / folder operations.
        //   *
        //   * Whether a client supports versioned document edits is expressed via
        //   * `workspace.workspaceEdit.documentChanges` client capability.
        //   *
        //   * If a client neither supports `documentChanges` nor
        //   * `workspace.workspaceEdit.resourceOperations` then only plain `TextEdit`s
        //   * using the `changes` property are supported.
        //   */
        //  documentChanges?: (
        //      TextDocumentEdit[] |
        //      (TextDocumentEdit | CreateFile | RenameFile | DeleteFile)[]
        //  );

        //  /**
        //   * A map of change annotations that can be referenced in
        //   * `AnnotatedTextEdit`s or create, rename and delete file / folder
        //   * operations.
        //   *
        //   * Whether clients honor this property depends on the client capability
        //   * `workspace.changeAnnotationSupport`.
        //   *
        //   * @since 3.16.0
        //   */
        //  changeAnnotations?: {
        //      [id: string /* ChangeAnnotationIdentifier */]: ChangeAnnotation;
        //  };
    };

#pragma region Document Sync

    inline constexpr const char* LSPMethod_DidOpenTextDocument   = "textDocument/didOpen";
    inline constexpr const char* LSPMethod_DidCloseTextDocument  = "textDocument/didClose";
    inline constexpr const char* LSPMethod_DidChangeTextDocument = "textDocument/didChange";

    // /**
    //  * Defines how the host (editor) should sync document changes to the language
    //  * server.
    //  */
    enum class TextDocumentSyncKind
    {
        // /**
        //  * Documents should not be synced at all.
        //  */
        // export const None = 0;
        None = 0,

        // /**
        //  * Documents are synced by always sending the full content
        //  * of the document.
        //  */
        // export const Full = 1;
        Full = 1,

        // /**
        //  * Documents are synced by sending the full content on open.
        //  * After that only incremental updates to the document are
        //  * sent.
        //  */
        // export const Incremental = 2;
        Incremental = 2,
    };

    struct TextDocumentSyncOptions
    {
        // /**
        //  * Open and close notifications are sent to the server. If omitted open
        //  * close notifications should not be sent.
        //  */
        // openClose?: boolean;
        bool openClose;

        // /**
        //  * Change notifications are sent to the server. See
        //  * TextDocumentSyncKind.None, TextDocumentSyncKind.Full and
        //  * TextDocumentSyncKind.Incremental. If omitted it defaults to
        //  * TextDocumentSyncKind.None.
        //  */
        // change?: TextDocumentSyncKind;
        TextDocumentSyncKind change;
    };

    struct DidOpenTextDocumentParams
    {
        // /**
        //  * The document that was opened.
        //  */
        // textDocument: TextDocumentItem;
        TextDocumentItem textDocument;
    };

    // Although the LSP defines TextDocumentContentChangeEvent as a union type, we use an optional flag on
    // TextDocumentContentChangeEvent::range to denote the difference. That is,
    // - Partial change if range has value
    // - Whole document change if range doesn't have value
    struct TextDocumentContentChangeEvent
    {
        // /**
        //  * The range of the document that changed.
        //  */
        // range: Range;
        std::optional<Range> range;

        // /**
        //  * The optional length of the range that got replaced.
        //  *
        //  * @deprecated use range instead.
        //  */
        // rangeLength?: uinteger;

        // /**
        //  * The new text for the provided range.
        //  */
        // text: string;
        // ---
        // /**
        //  * The new text of the whole document.
        //  */
        // text: string;
        std::string text;
    };

    // /**
    //  * An event describing a change to a text document. If only a text is provided
    //  * it is considered to be the full content of the document.
    //  */
    struct DidChangeTextDocumentParams
    {
        // /**
        //  * The document that did change. The version number points
        //  * to the version after all provided content changes have
        //  * been applied.
        //  */
        // textDocument: VersionedTextDocumentIdentifier;
        VersionedTextDocumentIdentifier textDocument;

        // /**
        //  * The actual content changes. The content changes describe single state
        //  * changes to the document. So if there are two content changes c1 (at
        //  * array index 0) and c2 (at array index 1) for a document in state S then
        //  * c1 moves the document from S to S' and c2 from S' to S''. So c1 is
        //  * computed on the state S and c2 is computed on the state S'.
        //  *
        //  * To mirror the content of a document using change events use the following
        //  * approach:
        //  * - start with the same initial content
        //  * - apply the 'textDocument/didChange' notifications in the order you
        //  *   receive them.
        //  * - apply the `TextDocumentContentChangeEvent`s in a single notification
        //  *   in the order you receive them.
        //  */
        // contentChanges: TextDocumentContentChangeEvent[];
        std::vector<TextDocumentContentChangeEvent> contentChanges;
    };

    struct DidCloseTextDocumentParams
    {
        // /**
        //  * The document that was closed.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };

#pragma endregion

#pragma region Show Message
    inline constexpr const char* LSPMethod_ShowMessage = "window/showMessage";

    enum class MessageType
    {
        // /**
        //  * An error message.
        //  */
        //  export const Error = 1;
        Error = 1,
        //  /**
        //   * A warning message.
        //   */
        //  export const Warning = 2;
        Warning = 2,
        //  /**
        //   * An information message.
        //   */
        //  export const Info = 3;
        Info = 3,
        //  /**
        //   * A log message.
        //   */
        //  export const Log = 4;
        Log = 4,
        //  /**
        //   * A debug message.
        //   *
        //   * @since 3.18.0
        //   * @proposed
        //   */
        //  export const Debug = 5;
        Debug = 5,
    };

    struct ShowMessageParams
    {
        // /**
        //  * The message type. See {@link MessageType}.
        //  */
        // type: MessageType;
        MessageType type;

        // /**
        //  * The actual message.
        //  */
        // message: string;
        std::string message;
    };
#pragma endregion

#pragma region Go to Declaration
    inline constexpr const char* LSPMethod_Declaration = "textDocument/declaration";

    struct DeclarationOptions
    {
    };

    struct DeclarationParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;
    };
#pragma endregion

#pragma region Go to Definition
    inline constexpr const char* LSPMethod_Definition = "textDocument/definition";

    struct DefinitionOptions
    {
    };

    struct DefinitionParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;
    };
#pragma endregion

#pragma region Find References
    inline constexpr const char* LSPMethod_References = "textDocument/references";

    struct ReferenceOptions
    {
    };

    struct ReferenceContext
    {
        // /**
        //  * Include the declaration of the current symbol.
        //  */
        //  includeDeclaration: boolean;
        bool includeDeclaration;
    };

    struct ReferenceParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;

        // context: ReferenceContext;
        ReferenceContext context;
    };
#pragma endregion

#pragma region Hover
    inline constexpr const char* LSPMethod_Hover = "textDocument/hover";

    struct HoverOptions
    {
    };

    struct HoverParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;
    };

    // /**
    //  * The result of a hover request.
    //  */
    struct Hover
    {
        // /**
        //  * The hover's content
        //  */
        // contents: MarkedString | MarkedString[] | MarkupContent;
        MarkupContent contents;

        // /**
        //  * An optional range is a range inside a text document
        //  * that is used to visualize a hover, e.g. by changing the background color.
        //  */
        // range?: Range;
        Range range;
    };
#pragma endregion

#pragma region Folding Range
    inline constexpr const char* LSPMethod_FoldingRange = "textDocument/foldingRange";

    struct FoldingRangeOptions
    {
    };

    struct FoldingRangeParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };

    // /**
    //  * Represents a folding range. To be valid, start and end line must be bigger
    //  * than zero and smaller than the number of lines in the document. Clients
    //  * are free to ignore invalid ranges.
    //  */
    struct FoldingRange
    {
        // /**
        //  * The zero-based start line of the range to fold. The folded area starts
        //  * after the line's last character. To be valid, the end must be zero or
        //  * larger and smaller than the number of lines in the document.
        //  */
        //  startLine: uinteger;
        uinteger startLine;

        //  /**
        //   * The zero-based character offset from where the folded range starts. If
        //   * not defined, defaults to the length of the start line.
        //   */
        //  startCharacter?: uinteger;

        //  /**
        //   * The zero-based end line of the range to fold. The folded area ends with
        //   * the line's last character. To be valid, the end must be zero or larger
        //   * and smaller than the number of lines in the document.
        //   */
        //  endLine: uinteger;
        uinteger endLine;

        //  /**
        //   * The zero-based character offset before the folded range ends. If not
        //   * defined, defaults to the length of the end line.
        //   */
        //  endCharacter?: uinteger;

        //  /**
        //   * Describes the kind of the folding range such as `comment` or `region`.
        //   * The kind is used to categorize folding ranges and used by commands like
        //   * 'Fold all comments'. See [FoldingRangeKind](#FoldingRangeKind) for an
        //   * enumeration of standardized kinds.
        //   */
        //  kind?: FoldingRangeKind;

        //  /**
        //   * The text that the client should show when the specified range is
        //   * collapsed. If not defined or not supported by the client, a default
        //   * will be chosen by the client.
        //   *
        //   * @since 3.17.0 - proposed
        //   */
        //  collapsedText?: string;
    };
#pragma endregion

#pragma region Document Symbols
    inline constexpr const char* LSPMethod_DocumentSymbol = "textDocument/documentSymbol";

    struct DocumentSymbolOptions
    {
    };

    struct DocumentSymbolParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };

    // /**
    //  * A symbol kind.
    //  */
    enum class SymbolKind
    {
        // export const File = 1;
        File = 1,
        // export const Module = 2;
        Module = 2,
        // export const Namespace = 3;
        Namespace = 3,
        // export const Package = 4;
        Package = 4,
        // export const Class = 5;
        Class = 5,
        // export const Method = 6;
        Method = 6,
        // export const Property = 7;
        Property = 7,
        // export const Field = 8;
        Field = 8,
        // export const Constructor = 9;
        Constructor = 9,
        // export const Enum = 10;
        Enum = 10,
        // export const Interface = 11;
        Interface = 11,
        // export const Function = 12;
        Function = 12,
        // export const Variable = 13;
        Variable = 13,
        // export const Constant = 14;
        Constant = 14,
        // export const String = 15;
        String = 15,
        // export const Number = 16;
        Number = 16,
        // export const Boolean = 17;
        Boolean = 17,
        // export const Array = 18;
        Array = 18,
        // export const Object = 19;
        Object = 19,
        // export const Key = 20;
        Key = 20,
        // export const Null = 21;
        Null = 21,
        // export const EnumMember = 22;
        EnumMember = 22,
        // export const Struct = 23;
        Struct = 23,
        // export const Event = 24;
        Event = 24,
        // export const Operator = 25;
        Operator = 25,
        // export const TypeParameter = 26;
        TypeParameter = 26,
    };

    // /**
    //  * Represents programming constructs like variables, classes, interfaces etc.
    //  * that appear in a document. Document symbols can be hierarchical and they
    //  * have two ranges: one that encloses its definition and one that points to its
    //  * most interesting range, e.g. the range of an identifier.
    //  */
    struct DocumentSymbol
    {
        // /**
        //  * The name of this symbol. Will be displayed in the user interface and
        //  * therefore must not be an empty string or a string only consisting of
        //  * white spaces.
        //  */
        //  name: string;
        std::string name;

        //  /**
        //   * More detail for this symbol, e.g the signature of a function.
        //   */
        //  detail?: string;
        std::string detail;

        //  /**
        //   * The kind of this symbol.
        //   */
        //  kind: SymbolKind;
        SymbolKind kind;

        //  /**
        //   * Tags for this document symbol.
        //   *
        //   * @since 3.16.0
        //   */
        //  tags?: SymbolTag[];

        //  /**
        //   * Indicates if this symbol is deprecated.
        //   *
        //   * @deprecated Use tags instead
        //   */
        //  deprecated?: boolean;

        //  /**
        //   * The range enclosing this symbol not including leading/trailing whitespace
        //   * but everything else like comments. This information is typically used to
        //   * determine if the clients cursor is inside the symbol to reveal in the
        //   * symbol in the UI.
        //   */
        //  range: Range;
        Range range;

        //  /**
        //   * The range that should be selected and revealed when this symbol is being
        //   * picked, e.g. the name of a function. Must be contained by the `range`.
        //   */
        //  selectionRange: Range;
        Range selectionRange;

        //  /**
        //   * Children of this symbol, e.g. properties of a class.
        //   */
        //  children?: DocumentSymbol[];
        std::vector<DocumentSymbol> children;
    };
#pragma endregion

#pragma region Semantic Tokens
    inline constexpr const char* LSPMethod_SemanticTokensFull      = "textDocument/semanticTokens/full";
    inline constexpr const char* LSPMethod_SemanticTokensFullDelta = "textDocument/semanticTokens/full/delta";

    struct SemanticTokensLegend
    {
        // /**
        //  * The token types a server uses.
        //  */
        // tokenTypes: string[];
        std::vector<std::string> tokenTypes;

        // /**
        //  * The token modifiers a server uses.
        //  */
        // tokenModifiers: string[];
        std::vector<std::string> tokenModifiers;
    };

    struct SemanticTokensOptions
    {
        // /**
        //  * The legend used by the server
        //  */
        //  legend: SemanticTokensLegend;
        SemanticTokensLegend legend;

        //  /**
        //   * Server supports providing semantic tokens for a specific range
        //   * of a document.
        //   */
        //  range?: boolean | {
        //  };

        //  /**
        //   * Server supports providing semantic tokens for a full document.
        //   */
        //  full?: boolean | {
        //      /**
        //       * The server supports deltas for full documents.
        //       */
        //      delta?: boolean;
        //  };
        struct FullOptions
        {
            // /**
            //  * The server supports deltas for full documents.
            //  */
            // delta?: boolean;
            bool delta;
        };
        FullOptions full;
    };

    struct SemanticTokensParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };

    struct SemanticTokens
    {
        // /**
        //  * An optional result id. If provided and clients support delta updating
        //  * the client will include the result id in the next semantic token request.
        //  * A server can then instead of computing all semantic tokens again simply
        //  * send a delta.
        //  */
        //  resultId?: string;
        std::string resultId;

        //  /**
        //   * The actual tokens.
        //   */
        //  data: uinteger[];
        std::vector<uinteger> data;
    };

    struct SemanticTokensDeltaParams
    {
        // /**
        //  * The text document.
        //  */
        //  textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        //  /**
        //   * The result id of a previous response. The result Id can either point to
        //   * a full response or a delta response depending on what was received last.
        //   */
        //  previousResultId: string;
        std::string previousResultId;
    };

    struct SemanticTokensEdit
    {
        // /**
        //  * The start offset of the edit.
        //  */
        //  start: uinteger;
        uinteger start;

        //  /**
        //   * The count of elements to remove.
        //   */
        //  deleteCount: uinteger;
        uinteger deleteCount;

        //  /**
        //   * The elements to insert.
        //   */
        //  data?: uinteger[];
        std::vector<uinteger> data;
    };

    struct SemanticTokensDelta
    {
        // readonly resultId?: string;
        std::string resultId;

        // /**
        //  * The semantic token edits to transform a previous result into a new
        //  * result.
        //  */
        // edits: SemanticTokensEdit[];
        std::vector<SemanticTokensEdit> edits;
    };
#pragma endregion

#pragma region Inlay Hint
    inline constexpr const char* LSPMethod_InlayHint = "textDocument/inlayHint";

    // /**
    //  * Inlay hint options used during static registration.
    //  *
    //  * @since 3.17.0
    //  */
    struct InlayHintOptions
    {
        // /**
        //  * The server provides support to resolve additional
        //  * information for an inlay hint item.
        //  */
        //  resolveProvider?: boolean;
        bool resolveProvider;
    };

    // /**
    //  * A parameter literal used in inlay hint requests.
    //  *
    //  * @since 3.17.0
    //  */
    struct InlayHintParams
    {
        // /**
        //  * The text document.
        //  */
        //  textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        //  /**
        //   * The visible document range for which inlay hints should be computed.
        //   */
        //  range: Range;
        Range range;
    };

    // /**
    //  * An inlay hint label part allows for interactive and composite labels
    //  * of inlay hints.
    //  *
    //  * @since 3.17.0
    //  */
    struct InlayHintLabelPart
    {
        // /**
        //  * The value of this label part.
        //  */
        //  value: string;
        std::string value;

        //  /**
        //   * The tooltip text when you hover over this label part. Depending on
        //   * the client capability `inlayHint.resolveSupport` clients might resolve
        //   * this property late using the resolve request.
        //   */
        //  tooltip?: string | MarkupContent;

        //  /**
        //   * An optional source code location that represents this
        //   * label part.
        //   *
        //   * The editor will use this location for the hover and for code navigation
        //   * features: This part will become a clickable link that resolves to the
        //   * definition of the symbol at the given location (not necessarily the
        //   * location itself), it shows the hover that shows at the given location,
        //   * and it shows a context menu with further code navigation commands.
        //   *
        //   * Depending on the client capability `inlayHint.resolveSupport` clients
        //   * might resolve this property late using the resolve request.
        //   */
        //  location?: Location;

        //  /**
        //   * An optional command for this label part.
        //   *
        //   * Depending on the client capability `inlayHint.resolveSupport` clients
        //   * might resolve this property late using the resolve request.
        //   */
        //  command?: Command;
    };

    // /**
    //  * Inlay hint information.
    //  *
    //  * @since 3.17.0
    //  */
    struct InlayHint
    {
        // /**
        //  * The position of this hint.
        //  *
        //  * If multiple hints have the same position, they will be shown in the order
        //  * they appear in the response.
        //  */
        //  position: Position;
        Position position;

        //  /**
        //   * The label of this hint. A human readable string or an array of
        //   * InlayHintLabelPart label parts.
        //   *
        //   * *Note* that neither the string nor the label part can be empty.
        //   */
        //  label: string | InlayHintLabelPart[];
        std::string label;

        //  /**
        //   * The kind of this hint. Can be omitted in which case the client
        //   * should fall back to a reasonable default.
        //   */
        //  kind?: InlayHintKind;

        //  /**
        //   * Optional text edits that are performed when accepting this inlay hint.
        //   *
        //   * *Note* that edits are expected to change the document so that the inlay
        //   * hint (or its nearest variant) is now part of the document and the inlay
        //   * hint itself is now obsolete.
        //   *
        //   * Depending on the client capability `inlayHint.resolveSupport` clients
        //   * might resolve this property late using the resolve request.
        //   */
        //  textEdits?: TextEdit[];

        //  /**
        //   * The tooltip text when you hover over this item.
        //   *
        //   * Depending on the client capability `inlayHint.resolveSupport` clients
        //   * might resolve this property late using the resolve request.
        //   */
        //  tooltip?: string | MarkupContent;

        //  /**
        //   * Render padding before the hint.
        //   *
        //   * Note: Padding should use the editor's background color, not the
        //   * background color of the hint itself. That means padding can be used
        //   * to visually align/separate an inlay hint.
        //   */
        //  paddingLeft?: boolean;
        bool paddingLeft;

        //  /**
        //   * Render padding after the hint.
        //   *
        //   * Note: Padding should use the editor's background color, not the
        //   * background color of the hint itself. That means padding can be used
        //   * to visually align/separate an inlay hint.
        //   */
        //  paddingRight?: boolean;
        bool paddingRight;

        //  /**
        //   * A data entry field that is preserved on an inlay hint between
        //   * a `textDocument/inlayHint` and a `inlayHint/resolve` request.
        //   */
        //  data?: LSPAny;
    };
#pragma endregion

#pragma region Completion Proposals
    inline constexpr const char* LSPMethod_Completion = "textDocument/completion";

    // /**
    //  * Completion options.
    //  */
    struct CompletionOptions
    {
        // /**
        //  * The additional characters, beyond the defaults provided by the client (typically
        //  * [a-zA-Z]), that should automatically trigger a completion request. For example
        //  * `.` in JavaScript represents the beginning of an object property or method and is
        //  * thus a good candidate for triggering a completion request.
        //  *
        //  * Most tools trigger a completion request automatically without explicitly
        //  * requesting it using a keyboard shortcut (e.g. Ctrl+Space). Typically they
        //  * do so when the user starts to type an identifier. For example if the user
        //  * types `c` in a JavaScript file code complete will automatically pop up
        //  * present `console` besides others as a completion item. Characters that
        //  * make up identifiers don't need to be listed here.
        //  */
        //  triggerCharacters?: string[];
        std::vector<std::string> triggerCharacters;

        //  /**
        //   * The list of all possible characters that commit a completion. This field
        //   * can be used if clients don't support individual commit characters per
        //   * completion item. See client capability
        //   * `completion.completionItem.commitCharactersSupport`.
        //   *
        //   * If a server provides both `allCommitCharacters` and commit characters on
        //   * an individual completion item the ones on the completion item win.
        //   *
        //   * @since 3.2.0
        //   */
        //  allCommitCharacters?: string[];

        //  /**
        //   * The server provides support to resolve additional
        //   * information for a completion item.
        //   */
        //  resolveProvider?: boolean;

        //  /**
        //   * The server supports the following `CompletionItem` specific
        //   * capabilities.
        //   *
        //   * @since 3.17.0
        //   */
        //  completionItem?: {
        //      /**
        //       * The server has support for completion item label
        //       * details (see also `CompletionItemLabelDetails`) when receiving
        //       * a completion item in a resolve call.
        //       *
        //       * @since 3.17.0
        //       */
        //      labelDetailsSupport?: boolean;
        //  }
    };

    // /**
    //  * How a completion was triggered
    //  */
    enum class CompletionTriggerKind
    {
        // /**
        //  * Completion was triggered by typing an identifier (24x7 code
        //  * complete), manual invocation (e.g Ctrl+Space) or via API.
        //  */
        //  export const Invoked: 1 = 1;
        Invoked = 1,

        //  /**
        //   * Completion was triggered by a trigger character specified by
        //   * the `triggerCharacters` properties of the
        //   * `CompletionRegistrationOptions`.
        //   */
        //  export const TriggerCharacter: 2 = 2;
        TriggerCharacter = 2,

        //  /**
        //   * Completion was re-triggered as the current completion list is incomplete.
        //   */
        //  export const TriggerForIncompleteCompletions: 3 = 3;
        TriggerForIncompleteCompletions = 3,
    };

    // /**
    //  * Contains additional information about the context in which a completion
    //  * request is triggered.
    //  */
    struct CompletionContext
    {
        // /**
        //  * How the completion was triggered.
        //  */
        //  triggerKind: CompletionTriggerKind;
        CompletionTriggerKind triggerKind;

        //  /**
        //   * The trigger character (a single character) that has trigger code
        //   * complete. Is undefined if
        //   * `triggerKind !== CompletionTriggerKind.TriggerCharacter`
        //   */
        //  triggerCharacter?: string;
    };

    struct CompletionParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;

        // /**
        //  * The completion context. This is only available if the client specifies
        //  * to send this using the client capability
        //  * `completion.contextSupport === true`
        //  */
        //  context?: CompletionContext;
        CompletionContext context;
    };

    // /**
    //  * The kind of a completion entry.
    //  */
    enum class CompletionItemKind
    {
        // export const Text = 1;
        Text = 1,
        // export const Method = 2;
        Method = 2,
        // export const Function = 3;
        Function = 3,
        // export const Constructor = 4;
        Constructor = 4,
        // export const Field = 5;
        Field = 5,
        // export const Variable = 6;
        Variable = 6,
        // export const Class = 7;
        Class = 7,
        // export const Interface = 8;
        Interface = 8,
        // export const Module = 9;
        Module = 9,
        // export const Property = 10;
        Property = 10,
        // export const Unit = 11;
        Unit = 11,
        // export const Value = 12;
        Value = 12,
        // export const Enum = 13;
        Enum = 13,
        // export const Keyword = 14;
        Keyword = 14,
        // export const Snippet = 15;
        Snippet = 15,
        // export const Color = 16;
        Color = 16,
        // export const File = 17;
        File = 17,
        // export const Reference = 18;
        Reference = 18,
        // export const Folder = 19;
        Folder = 19,
        // export const EnumMember = 20;
        EnumMember = 20,
        // export const Constant = 21;
        Constant = 21,
        // export const Struct = 22;
        Struct = 22,
        // export const Event = 23;
        Event = 23,
        // export const Operator = 24;
        Operator = 24,
        // export const TypeParameter = 25;
        TypeParameter = 25,
    };

    struct CompletionItem
    {
        // /**
        //  * The label of this completion item.
        //  *
        //  * The label property is also by default the text that
        //  * is inserted when selecting this completion.
        //  *
        //  * If label details are provided the label itself should
        //  * be an unqualified name of the completion item.
        //  */
        //  label: string;
        std::string label;

        //  /**
        //   * Additional details for the label
        //   *
        //   * @since 3.17.0
        //   */
        //  labelDetails?: CompletionItemLabelDetails;

        //  /**
        //   * The kind of this completion item. Based of the kind
        //   * an icon is chosen by the editor. The standardized set
        //   * of available values is defined in `CompletionItemKind`.
        //   */
        //  kind?: CompletionItemKind;
        CompletionItemKind kind;

        //  /**
        //   * Tags for this completion item.
        //   *
        //   * @since 3.15.0
        //   */
        //  tags?: CompletionItemTag[];

        //  /**
        //   * A human-readable string with additional information
        //   * about this item, like type or symbol information.
        //   */
        //  detail?: string;

        //  /**
        //   * A human-readable string that represents a doc-comment.
        //   */
        //  documentation?: string | MarkupContent;

        //  /**
        //   * Indicates if this item is deprecated.
        //   *
        //   * @deprecated Use `tags` instead if supported.
        //   */
        //  deprecated?: boolean;

        //  /**
        //   * Select this item when showing.
        //   *
        //   * *Note* that only one completion item can be selected and that the
        //   * tool / client decides which item that is. The rule is that the *first*
        //   * item of those that match best is selected.
        //   */
        //  preselect?: boolean;

        //  /**
        //   * A string that should be used when comparing this item
        //   * with other items. When omitted the label is used
        //   * as the sort text for this item.
        //   */
        //  sortText?: string;

        //  /**
        //   * A string that should be used when filtering a set of
        //   * completion items. When omitted the label is used as the
        //   * filter text for this item.
        //   */
        //  filterText?: string;

        //  /**
        //   * A string that should be inserted into a document when selecting
        //   * this completion. When omitted the label is used as the insert text
        //   * for this item.
        //   *
        //   * The `insertText` is subject to interpretation by the client side.
        //   * Some tools might not take the string literally. For example
        //   * VS Code when code complete is requested in this example
        //   * `con<cursor position>` and a completion item with an `insertText` of
        //   * `console` is provided it will only insert `sole`. Therefore it is
        //   * recommended to use `textEdit` instead since it avoids additional client
        //   * side interpretation.
        //   */
        //  insertText?: string;

        //  /**
        //   * The format of the insert text. The format applies to both the
        //   * `insertText` property and the `newText` property of a provided
        //   * `textEdit`. If omitted defaults to `InsertTextFormat.PlainText`.
        //   *
        //   * Please note that the insertTextFormat doesn't apply to
        //   * `additionalTextEdits`.
        //   */
        //  insertTextFormat?: InsertTextFormat;

        //  /**
        //   * How whitespace and indentation is handled during completion
        //   * item insertion. If not provided the client's default value depends on
        //   * the `textDocument.completion.insertTextMode` client capability.
        //   *
        //   * @since 3.16.0
        //   * @since 3.17.0 - support for `textDocument.completion.insertTextMode`
        //   */
        //  insertTextMode?: InsertTextMode;

        //  /**
        //   * An edit which is applied to a document when selecting this completion.
        //   * When an edit is provided the value of `insertText` is ignored.
        //   *
        //   * *Note:* The range of the edit must be a single line range and it must
        //   * contain the position at which completion has been requested.
        //   *
        //   * Most editors support two different operations when accepting a completion
        //   * item. One is to insert a completion text and the other is to replace an
        //   * existing text with a completion text. Since this can usually not be
        //   * predetermined by a server it can report both ranges. Clients need to
        //   * signal support for `InsertReplaceEdit`s via the
        //   * `textDocument.completion.completionItem.insertReplaceSupport` client
        //   * capability property.
        //   *
        //   * *Note 1:* The text edit's range as well as both ranges from an insert
        //   * replace edit must be a [single line] and they must contain the position
        //   * at which completion has been requested.
        //   * *Note 2:* If an `InsertReplaceEdit` is returned the edit's insert range
        //   * must be a prefix of the edit's replace range, that means it must be
        //   * contained and starting at the same position.
        //   *
        //   * @since 3.16.0 additional type `InsertReplaceEdit`
        //   */
        //  textEdit?: TextEdit | InsertReplaceEdit;

        //  /**
        //   * The edit text used if the completion item is part of a CompletionList and
        //   * CompletionList defines an item default for the text edit range.
        //   *
        //   * Clients will only honor this property if they opt into completion list
        //   * item defaults using the capability `completionList.itemDefaults`.
        //   *
        //   * If not provided and a list's default range is provided the label
        //   * property is used as a text.
        //   *
        //   * @since 3.17.0
        //   */
        //  textEditText?: string;

        //  /**
        //   * An optional array of additional text edits that are applied when
        //   * selecting this completion. Edits must not overlap (including the same
        //   * insert position) with the main edit nor with themselves.
        //   *
        //   * Additional text edits should be used to change text unrelated to the
        //   * current cursor position (for example adding an import statement at the
        //   * top of the file if the completion item will insert an unqualified type).
        //   */
        //  additionalTextEdits?: TextEdit[];

        //  /**
        //   * An optional set of characters that when pressed while this completion is
        //   * active will accept it first and then type that character. *Note* that all
        //   * commit characters should have `length=1` and that superfluous characters
        //   * will be ignored.
        //   */
        //  commitCharacters?: string[];

        //  /**
        //   * An optional command that is executed *after* inserting this completion.
        //   * *Note* that additional modifications to the current document should be
        //   * described with the additionalTextEdits-property.
        //   */
        //  command?: Command;

        //  /**
        //   * A data entry field that is preserved on a completion item between
        //   * a completion and a completion resolve request.
        //   */
        //  data?: LSPAny;
    };

    // /**
    //  * Represents a collection of [completion items](#CompletionItem) to be
    //  * presented in the editor.
    //  */
    struct CompletionList
    {
        // /**
        //  * This list is not complete. Further typing should result in recomputing
        //  * this list.
        //  *
        //  * Recomputed lists have all their items replaced (not appended) in the
        //  * incomplete completion sessions.
        //  */
        // isIncomplete: boolean;
        bool isIncomplete;

        // /**
        //  * In many cases the items of an actual completion result share the same
        //  * value for properties like `commitCharacters` or the range of a text
        //  * edit. A completion list can therefore define item defaults which will
        //  * be used if a completion item itself doesn't specify the value.
        //  *
        //  * If a completion list specifies a default value and a completion item
        //  * also specifies a corresponding value the one from the item is used.
        //  *
        //  * Servers are only allowed to return default values if the client
        //  * signals support for this via the `completionList.itemDefaults`
        //  * capability.
        //  *
        //  * @since 3.17.0
        //  */
        // itemDefaults?: { ... }

        // /**
        //  * The completion items.
        //  */
        // items : CompletionItem[];
        std::vector<CompletionItem> items;
    };

#pragma endregion

#pragma region Signature Help
    inline constexpr const char* LSPMethod_SignatureHelp = "textDocument/signatureHelp";

    struct SignatureHelpOptions
    {
        // /**
        //  * The characters that trigger signature help
        //  * automatically.
        //  */
        //  triggerCharacters?: string[];
        std::vector<std::string> triggerCharacters;

        //  /**
        //   * List of characters that re-trigger signature help.
        //   *
        //   * These trigger characters are only active when signature help is already
        //   * showing. All trigger characters are also counted as re-trigger
        //   * characters.
        //   *
        //   * @since 3.15.0
        //   */
        //  retriggerCharacters?: string[];
        std::vector<std::string> retriggerCharacters;
    };

    struct SignatureHelpParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The position inside the text document.
        //  */
        // position: Position;
        Position position;

        // /**
        //  * The signature help context. This is only available if the client
        //  * specifies to send this using the client capability
        //  * `textDocument.signatureHelp.contextSupport === true`
        //  *
        //  * @since 3.15.0
        //  */
        // context?: SignatureHelpContext;
    };

    // /**
    //  * Represents a parameter of a callable-signature. A parameter can
    //  * have a label and a doc-comment.
    //  */
    struct ParameterInformation
    {
        // /**
        //  * The label of this parameter information.
        //  *
        //  * Either a string or an inclusive start and exclusive end offsets within
        //  * its containing signature label. (see SignatureInformation.label). The
        //  * offsets are based on a UTF-16 string representation as `Position` and
        //  * `Range` does.
        //  *
        //  * *Note*: a label of type string should be a substring of its containing
        //  * signature label. Its intended use case is to highlight the parameter
        //  * label part in the `SignatureInformation.label`.
        //  */
        //  label: string | [uinteger, uinteger];
        std::string label;

        //  /**
        //   * The human-readable doc-comment of this parameter. Will be shown
        //   * in the UI but can be omitted.
        //   */
        //  documentation?: string | MarkupContent;
    };

    // /**
    //  * Represents the signature of something callable. A signature
    //  * can have a label, like a function-name, a doc-comment, and
    //  * a set of parameters.
    //  */
    struct SignatureInformation
    {
        // /**
        //  * The label of this signature. Will be shown in
        //  * the UI.
        //  */
        // label: string;
        std::string label;

        // /**
        //  * The human-readable doc-comment of this signature. Will be shown
        //  * in the UI but can be omitted.
        //  */
        // documentation?: string | MarkupContent;
        std::string documentation;

        // /**
        //  * The parameters of this signature.
        //  */
        // parameters?: ParameterInformation[];
        std::vector<ParameterInformation> parameters;

        // /**
        //  * The index of the active parameter.
        //  *
        //  * If provided, this is used in place of `SignatureHelp.activeParameter`.
        //  *
        //  * @since 3.16.0
        //  */
        // activeParameter?: uinteger;
    };

    // /**
    //  * Signature help represents the signature of something
    //  * callable. There can be multiple signature but only one
    //  * active and only one active parameter.
    //  */
    struct SignatureHelp
    {
        // /**
        //  * One or more signatures. If no signatures are available the signature help
        //  * request should return `null`.
        //  */
        //  signatures: SignatureInformation[];
        std::vector<SignatureInformation> signatures;

        //  /**
        //   * The active signature. If omitted or the value lies outside the
        //   * range of `signatures` the value defaults to zero or is ignore if
        //   * the `SignatureHelp` as no signatures.
        //   *
        //   * Whenever possible implementors should make an active decision about
        //   * the active signature and shouldn't rely on a default value.
        //   *
        //   * In future version of the protocol this property might become
        //   * mandatory to better express this.
        //   */
        //  activeSignature?: uinteger;

        //  /**
        //   * The active parameter of the active signature. If omitted or the value
        //   * lies outside the range of `signatures[activeSignature].parameters`
        //   * defaults to 0 if the active signature has parameters. If
        //   * the active signature has no parameters it is ignored.
        //   * In future version of the protocol this property might become
        //   * mandatory to better express the active parameter if the
        //   * active signature does have any.
        //   */
        //  activeParameter?: uinteger;
    };
#pragma endregion

#pragma region Publish Diagnostic
    inline constexpr const char* LSPMethod_PublishDiagnostic = "textDocument/publishDiagnostics";

    enum class DiagnosticSeverity
    {
        // /**
        //  * Reports an error.
        //  */
        // export const Error: 1 = 1;
        Error = 1,
        // /**
        //  * Reports a warning.
        //  */
        // export const Warning: 2 = 2;
        Warning = 2,
        // /**
        //  * Reports an information.
        //  */
        // export const Information: 3 = 3;
        Information = 3,
        // /**
        //  * Reports a hint.
        //  */
        // export const Hint: 4 = 4;
        Hint = 4,
    };

    struct Diagnostic
    {
        // /**
        //  * The range at which the message applies.
        //  */
        //  range: Range;
        Range range;

        //  /**
        //   * The diagnostic's severity. To avoid interpretation mismatches when a
        //   * server is used with different clients it is highly recommended that
        //   * servers always provide a severity value. If omitted, it’s recommended
        //   * for the client to interpret it as an Error severity.
        //   */
        //  severity?: DiagnosticSeverity;
        DiagnosticSeverity severity;

        //  /**
        //   * The diagnostic's code, which might appear in the user interface.
        //   */
        //  code?: integer | string;

        //  /**
        //   * An optional property to describe the error code.
        //   *
        //   * @since 3.16.0
        //   */
        //  codeDescription?: CodeDescription;

        //  /**
        //   * A human-readable string describing the source of this
        //   * diagnostic, e.g. 'typescript' or 'super lint'.
        //   */
        //  source?: string;

        //  /**
        //   * The diagnostic's message.
        //   */
        //  message: string;
        std::string message;

        //  /**
        //   * Additional metadata about the diagnostic.
        //   *
        //   * @since 3.15.0
        //   */
        //  tags?: DiagnosticTag[];

        //  /**
        //   * An array of related diagnostic information, e.g. when symbol-names within
        //   * a scope collide all definitions can be marked via this property.
        //   */
        //  relatedInformation?: DiagnosticRelatedInformation[];

        //  /**
        //   * A data entry field that is preserved between a
        //   * `textDocument/publishDiagnostics` notification and
        //   * `textDocument/codeAction` request.
        //   *
        //   * @since 3.16.0
        //   */
        //  data?: LSPAny;
    };

    struct PublishDiagnosticParams
    {
        // /**
        //  * The URI for which diagnostic information is reported.
        //  */
        // uri: DocumentUri;
        DocumentUri uri;

        // /**
        //  * Optional the version number of the document the diagnostics are published
        //  * for.
        //  *
        //  * @since 3.15.0
        //  */
        // version?: integer;
        int version;

        // /**
        //  * An array of diagnostic information items.
        //  */
        // diagnostics: Diagnostic[];
        std::vector<Diagnostic> diagnostics;
    };
#pragma endregion

#pragma region Inactive Region Extension
    inline constexpr const char* LSPMethod_PublishInactiveRegion = "glsld/textDocument/publishInactiveRegion";

    struct PublishInactiveRegionParams
    {
        // The URI for which inactive region information is reported.
        DocumentUri uri;

        // The line numbers of inactive regions. Every [2*i] is the start line and [2*i+1] is the end line.
        std::vector<lsp::uinteger> regions;
    };

#pragma endregion

#pragma region Lifecycle
    inline constexpr const char* LSPMethod_Initialize  = "initialize";
    inline constexpr const char* LSPMethod_Initialized = "initialized";
    inline constexpr const char* LSPMethod_SetTrace    = "$/setTrace";
    inline constexpr const char* LSPMethod_LogTrace    = "$/logTrace";
    inline constexpr const char* LSPMethod_Shutdown    = "shutdown";
    inline constexpr const char* LSPMethod_Exit        = "exit";

    struct InitializationOptions
    {
        std::optional<bool> supportGlsldExtensions;
    };

    struct InitializeParams
    {
        // /**
        //  * The process Id of the parent process that started the server. Is null if
        //  * the process has not been started by another process. If the parent
        //  * process is not alive then the server should exit (see exit notification)
        //  * its process.
        //  */
        //  processId: integer | null;

        //  /**
        //   * Information about the client
        //   *
        //   * @since 3.15.0
        //   */
        //  clientInfo?: {
        //      /**
        //       * The name of the client as defined by the client.
        //       */
        //      name: string;

        //      /**
        //       * The client's version as defined by the client.
        //       */
        //      version?: string;
        //  };

        //  /**
        //   * The locale the client is currently showing the user interface
        //   * in. This must not necessarily be the locale of the operating
        //   * system.
        //   *
        //   * Uses IETF language tags as the value's syntax
        //   * (See https://en.wikipedia.org/wiki/IETF_language_tag)
        //   *
        //   * @since 3.16.0
        //   */
        //  locale?: string;

        //  /**
        //   * The rootPath of the workspace. Is null
        //   * if no folder is open.
        //   *
        //   * @deprecated in favour of `rootUri`.
        //   */
        //  rootPath?: string | null;

        //  /**
        //   * The rootUri of the workspace. Is null if no
        //   * folder is open. If both `rootPath` and `rootUri` are set
        //   * `rootUri` wins.
        //   *
        //   * @deprecated in favour of `workspaceFolders`
        //   */
        //  rootUri: DocumentUri | null;

        //  /**
        //   * User provided initialization options.
        //   */
        //  initializationOptions?: LSPAny;
        std::optional<InitializationOptions> initializationOptions;

        //  /**
        //   * The capabilities provided by the client (editor or tool)
        //   */
        //  capabilities: ClientCapabilities;

        //  /**
        //   * The initial trace setting. If omitted trace is disabled ('off').
        //   */
        //  trace?: TraceValue;

        //  /**
        //   * The workspace folders configured in the client when the server starts.
        //   * This property is only available if the client supports workspace folders.
        //   * It can be `null` if the client supports workspace folders but none are
        //   * configured.
        //   *
        //   * @since 3.6.0
        //   */
        //  workspaceFolders?: WorkspaceFolder[] | null;
    };

    struct ServerCapabilities
    {
        // /**
        //  * The position encoding the server picked from the encodings offered
        //  * by the client via the client capability `general.positionEncodings`.
        //  *
        //  * If the client didn't provide any position encodings the only valid
        //  * value that a server can return is 'utf-16'.
        //  *
        //  * If omitted it defaults to 'utf-16'.
        //  *
        //  * @since 3.17.0
        //  */
        //  positionEncoding?: PositionEncodingKind;

        //  /**
        //   * Defines how text documents are synced. Is either a detailed structure
        //   * defining each notification or for backwards compatibility the
        //   * TextDocumentSyncKind number. If omitted it defaults to
        //   * `TextDocumentSyncKind.None`.
        //   */
        //  textDocumentSync?: TextDocumentSyncOptions | TextDocumentSyncKind;
        TextDocumentSyncOptions textDocumentSync;

        //  /**
        //   * Defines how notebook documents are synced.
        //   *
        //   * @since 3.17.0
        //   */
        //  notebookDocumentSync?: NotebookDocumentSyncOptions
        //      | NotebookDocumentSyncRegistrationOptions;

        //  /**
        //   * The server provides completion support.
        //   */
        //  completionProvider?: CompletionOptions;
        std::optional<CompletionOptions> completionProvider;

        //  /**
        //   * The server provides hover support.
        //   */
        //  hoverProvider?: boolean | HoverOptions;
        std::optional<HoverOptions> hoverProvider;

        //  /**
        //   * The server provides signature help support.
        //   */
        //  signatureHelpProvider?: SignatureHelpOptions;
        std::optional<SignatureHelpOptions> signatureHelpProvider;

        //  /**
        //   * The server provides go to declaration support.
        //   *
        //   * @since 3.14.0
        //   */
        //  declarationProvider?: boolean | DeclarationOptions
        //      | DeclarationRegistrationOptions;
        std::optional<DeclarationOptions> declarationProvider;

        //  /**
        //   * The server provides goto definition support.
        //   */
        //  definitionProvider?: boolean | DefinitionOptions;
        std::optional<DefinitionOptions> definitionProvider;

        //  /**
        //   * The server provides goto type definition support.
        //   *
        //   * @since 3.6.0
        //   */
        //  typeDefinitionProvider?: boolean | TypeDefinitionOptions
        //      | TypeDefinitionRegistrationOptions;

        //  /**
        //   * The server provides goto implementation support.
        //   *
        //   * @since 3.6.0
        //   */
        //  implementationProvider?: boolean | ImplementationOptions
        //      | ImplementationRegistrationOptions;

        //  /**
        //   * The server provides find references support.
        //   */
        //  referencesProvider?: boolean | ReferenceOptions;
        std::optional<ReferenceOptions> referencesProvider;

        //  /**
        //   * The server provides document highlight support.
        //   */
        //  documentHighlightProvider?: boolean | DocumentHighlightOptions;

        //  /**
        //   * The server provides document symbol support.
        //   */
        //  documentSymbolProvider?: boolean | DocumentSymbolOptions;
        std::optional<DocumentSymbolOptions> documentSymbolProvider;

        //  /**
        //   * The server provides code actions. The `CodeActionOptions` return type is
        //   * only valid if the client signals code action literal support via the
        //   * property `textDocument.codeAction.codeActionLiteralSupport`.
        //   */
        //  codeActionProvider?: boolean | CodeActionOptions;

        //  /**
        //   * The server provides code lens.
        //   */
        //  codeLensProvider?: CodeLensOptions;

        //  /**
        //   * The server provides document link support.
        //   */
        //  documentLinkProvider?: DocumentLinkOptions;

        //  /**
        //   * The server provides color provider support.
        //   *
        //   * @since 3.6.0
        //   */
        //  colorProvider?: boolean | DocumentColorOptions
        //      | DocumentColorRegistrationOptions;

        //  /**
        //   * The server provides document formatting.
        //   */
        //  documentFormattingProvider?: boolean | DocumentFormattingOptions;

        //  /**
        //   * The server provides document range formatting.
        //   */
        //  documentRangeFormattingProvider?: boolean | DocumentRangeFormattingOptions;

        //  /**
        //   * The server provides document formatting on typing.
        //   */
        //  documentOnTypeFormattingProvider?: DocumentOnTypeFormattingOptions;

        //  /**
        //   * The server provides rename support. RenameOptions may only be
        //   * specified if the client states that it supports
        //   * `prepareSupport` in its initial `initialize` request.
        //   */
        //  renameProvider?: boolean | RenameOptions;

        //  /**
        //   * The server provides folding provider support.
        //   *
        //   * @since 3.10.0
        //   */
        //  foldingRangeProvider?: boolean | FoldingRangeOptions
        //      | FoldingRangeRegistrationOptions;
        std::optional<FoldingRangeOptions> foldingRangeProvider;

        //  /**
        //   * The server provides execute command support.
        //   */
        //  executeCommandProvider?: ExecuteCommandOptions;

        //  /**
        //   * The server provides selection range support.
        //   *
        //   * @since 3.15.0
        //   */
        //  selectionRangeProvider?: boolean | SelectionRangeOptions
        //      | SelectionRangeRegistrationOptions;

        //  /**
        //   * The server provides linked editing range support.
        //   *
        //   * @since 3.16.0
        //   */
        //  linkedEditingRangeProvider?: boolean | LinkedEditingRangeOptions
        //      | LinkedEditingRangeRegistrationOptions;

        //  /**
        //   * The server provides call hierarchy support.
        //   *
        //   * @since 3.16.0
        //   */
        //  callHierarchyProvider?: boolean | CallHierarchyOptions
        //      | CallHierarchyRegistrationOptions;

        //  /**
        //   * The server provides semantic tokens support.
        //   *
        //   * @since 3.16.0
        //   */
        //  semanticTokensProvider?: SemanticTokensOptions
        //      | SemanticTokensRegistrationOptions;
        std::optional<SemanticTokensOptions> semanticTokensProvider;

        //  /**
        //   * Whether server provides moniker support.
        //   *
        //   * @since 3.16.0
        //   */
        //  monikerProvider?: boolean | MonikerOptions | MonikerRegistrationOptions;

        //  /**
        //   * The server provides type hierarchy support.
        //   *
        //   * @since 3.17.0
        //   */
        //  typeHierarchyProvider?: boolean | TypeHierarchyOptions
        //       | TypeHierarchyRegistrationOptions;

        //  /**
        //   * The server provides inline values.
        //   *
        //   * @since 3.17.0
        //   */
        //  inlineValueProvider?: boolean | InlineValueOptions
        //       | InlineValueRegistrationOptions;

        //  /**
        //   * The server provides inlay hints.
        //   *
        //   * @since 3.17.0
        //   */
        //  inlayHintProvider?: boolean | InlayHintOptions
        //       | InlayHintRegistrationOptions;
        std::optional<InlayHintOptions> inlayHintProvider;

        //  /**
        //   * The server has support for pull model diagnostics.
        //   *
        //   * @since 3.17.0
        //   */
        //  diagnosticProvider?: DiagnosticOptions | DiagnosticRegistrationOptions;

        //  /**
        //   * The server provides workspace symbol support.
        //   */
        //  workspaceSymbolProvider?: boolean | WorkspaceSymbolOptions;

        //  /**
        //   * Workspace specific server capabilities
        //   */
        //  workspace?: {
        //      /**
        //       * The server supports workspace folder.
        //       *
        //       * @since 3.6.0
        //       */
        //      workspaceFolders?: WorkspaceFoldersServerCapabilities;

        //      /**
        //       * The server is interested in file notifications/requests.
        //       *
        //       * @since 3.16.0
        //       */
        //      fileOperations?: {
        //          /**
        //           * The server is interested in receiving didCreateFiles
        //           * notifications.
        //           */
        //          didCreate?: FileOperationRegistrationOptions;

        //          /**
        //           * The server is interested in receiving willCreateFiles requests.
        //           */
        //          willCreate?: FileOperationRegistrationOptions;

        //          /**
        //           * The server is interested in receiving didRenameFiles
        //           * notifications.
        //           */
        //          didRename?: FileOperationRegistrationOptions;

        //          /**
        //           * The server is interested in receiving willRenameFiles requests.
        //           */
        //          willRename?: FileOperationRegistrationOptions;

        //          /**
        //           * The server is interested in receiving didDeleteFiles file
        //           * notifications.
        //           */
        //          didDelete?: FileOperationRegistrationOptions;

        //          /**
        //           * The server is interested in receiving willDeleteFiles file
        //           * requests.
        //           */
        //          willDelete?: FileOperationRegistrationOptions;
        //      };
        //  };

        //  /**
        //   * Experimental server capabilities.
        //   */
        //  experimental?: LSPAny;
    };

    struct InitializeResult
    {
        // /**
        //  * The capabilities the language server provides.
        //  */
        //  capabilities: ServerCapabilities;
        ServerCapabilities capabilities;

        //  /**
        //   * Information about the server.
        //   *
        //   * @since 3.15.0
        //   */
        //  serverInfo?: {
        //      /**
        //       * The name of the server as defined by the server.
        //       */
        //      name: string;
        //
        //      /**
        //       * The server's version as defined by the server.
        //       */
        //      version?: string;
        //  };
        struct ServerInfo
        {
            std::string name;
        };
        ServerInfo serverInfo;
    };

    struct InitializeError
    {
        // /**
        //  * Indicates whether the client execute the following retry logic:
        //  * (1) show the message provided by the ResponseError to the user
        //  * (2) user selects retry or cancel
        //  * (3) if user selected retry the initialize method is sent again.
        //  */
        // retry: boolean;
        bool retry;
    };

    struct InitializedParams
    {
    };

    struct SetTraceParams
    {
        // /**
        //  * The new value that should be assigned to the trace setting.
        //  */
        // value : TraceValue;
    };
#pragma endregion
} // namespace glsld::lsp