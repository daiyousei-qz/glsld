#pragma once

#include "ObjectMapper.h"

#include <cstdint>
#include <string>
#include <variant>
#include <any>
#include <optional>
#include <vector>

namespace glsld::lsp
{
    // using integer  = int32_t;
    // using uinteger = uint32_t;
    // using decimal  = float;

    using TextDocumentIdentifier = std::string;
    using Uri                    = std::string;
    using DocumentUri            = Uri;

    struct VersionedTextDocumentIdentifier
    {
        /**
         * The text document's URI.
         */
        DocumentUri uri;

        /**
         * The version number of this document.
         *
         * The version number of a document will increase after each change,
         * including undo/redo. The number doesn't need to be consecutive.
         */
        // version: integer;
        int32_t version;
    };

    struct RequestMessage
    {
        std::string id;
        std::string method;
        std::any params;
    };

    struct ResponseError
    {
        int32_t code;
        std::string message;
        std::any data;
    };

    struct ResponseMessage
    {
        std::string id;
        std::any result;
    };

    struct Position
    {
        // /**
        //  * Line position in a document (zero-based).
        //  */
        // line: uinteger;
        uint32_t line;

        // /**
        //  * Character offset on a line in a document (zero-based). The meaning of this
        //  * offset is determined by the negotiated `PositionEncodingKind`.
        //  *
        //  * If the character value is greater than the line length it defaults back
        //  * to the line length.
        //  */
        // character: uinteger;
        uint32_t character;
    };
    inline auto MapJson(JsonToObjectMapper& mapper, const Position& value) -> bool
    {
        if (!mapper.Map("line", value.line)) {
            return false;
        }
        if (!mapper.Map("character", value.character)) {
            return false;
        }

        return true;
    }

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
    inline auto MapJson(JsonToObjectMapper& mapper, const Range& value) -> bool
    {
        if (!mapper.Map("start", value.start)) {
            return false;
        }
        if (!mapper.Map("end", value.end)) {
            return false;
        }

        return true;
    }

#pragma region Show Message

    inline constexpr const char* LSPMethod_ShowMessage = "window/showMessage";

    enum class MessageType
    {
        // /**
        //  * An error message.
        //  */
        // export const Error = 1;
        Error = 1,
        // /**
        //  * A warning message.
        //  */
        // export const Warning = 2;
        Warning = 2,
        // /**
        //  * An information message.
        //  */
        // export const Info = 3;
        Info = 3,
        // /**
        //  * A log message.
        //  */
        // export const Log = 4;
        Log = 4,
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
    inline auto MapJson(JsonToObjectMapper& mapper, const ShowMessageParams& value) -> bool
    {
        if (!mapper.Map("type", static_cast<int32_t>(value.type))) {
            return false;
        }
        if (!mapper.Map("message", value.message)) {
            return false;
        }

        return true;
    }

#pragma endregion

#pragma region Document Symbol

    //
    // Document Symbol
    //

    inline constexpr const char* LSPMethod_DocumentSymbol = "textDocument/documentSymbol";

    struct DocumentSymbolClientCapabilities
    {
        // /**
        //  * Whether document symbol supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * Specific capabilities for the `SymbolKind` in the
        //  * `textDocument/documentSymbol` request.
        //  */
        // symbolKind?: {
        // 	/**
        // 	 * The symbol kind values the client supports. When this
        // 	 * property exists the client also guarantees that it will
        // 	 * handle values outside its set gracefully and falls back
        // 	 * to a default value when unknown.
        // 	 *
        // 	 * If this property is not present the client only supports
        // 	 * the symbol kinds from `File` to `Array` as defined in
        // 	 * the initial version of the protocol.
        // 	 */
        // 	valueSet?: SymbolKind[];
        // };

        // /**
        //  * The client supports hierarchical document symbols.
        //  */
        // hierarchicalDocumentSymbolSupport?: boolean;

        // /**
        //  * The client supports tags on `SymbolInformation`. Tags are supported on
        //  * `DocumentSymbol` if `hierarchicalDocumentSymbolSupport` is set to true.
        //  * Clients supporting tags have to handle unknown tags gracefully.
        //  *
        //  * @since 3.16.0
        //  */
        // tagSupport?: {
        // 	/**
        // 	 * The tags supported by the client.
        // 	 */
        // 	valueSet: SymbolTag[];
        // };

        // /**
        //  * The client supports an additional label presented in the UI when
        //  * registering a document symbol provider.
        //  *
        //  * @since 3.16.0
        //  */
        // labelSupport?: boolean;
    };

    struct DocumentSymbolParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, DocumentSymbolParams& value) -> bool
    {
        if (!mapper.Map("textDocument", value.textDocument)) {
            return false;
        }

        return true;
    }

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
    //  * Symbol tags are extra annotations that tweak the rendering of a symbol.
    //  *
    //  * @since 3.16
    //  */
    enum class SymbolTag
    {
        // /**
        //  * Render a symbol as obsolete, usually using a strike-out.
        //  */
        // export const Deprecated: 1 = 1;
        Deprecated = 1,
    };

    struct DocumentSymbol
    {
        // /**
        //  * The name of this symbol. Will be displayed in the user interface and
        //  * therefore must not be an empty string or a string only consisting of
        //  * white spaces.
        //  */
        // name: string;
        std::string name;

        // /**
        //  * More detail for this symbol, e.g the signature of a function.
        //  */
        // detail?: string;

        // /**
        //  * The kind of this symbol.
        //  */
        // kind: SymbolKind;
        SymbolKind kind;

        // /**
        //  * Tags for this document symbol.
        //  *
        //  * @since 3.16.0
        //  */
        // tags?: SymbolTag[];

        // /**
        //  * Indicates if this symbol is deprecated.
        //  *
        //  * @deprecated Use tags instead
        //  */
        // deprecated?: boolean;

        // /**
        //  * The range enclosing this symbol not including leading/trailing whitespace
        //  * but everything else like comments. This information is typically used to
        //  * determine if the clients cursor is inside the symbol to reveal in the
        //  * symbol in the UI.
        //  */
        // range: Range;
        Range range;

        // /**
        //  * The range that should be selected and revealed when this symbol is being
        //  * picked, e.g. the name of a function. Must be contained by the `range`.
        //  */
        // selectionRange: Range;
        Range selectionRange;

        // /**
        //  * Children of this symbol, e.g. properties of a class.
        //  */
        // children?: DocumentSymbol[];
    };
    inline auto MapJson(JsonToObjectMapper& mapper, const DocumentSymbol& value) -> bool
    {
        if (!mapper.Map("name", value.name)) {
            return false;
        }
        if (!mapper.Map("kind", static_cast<int32_t>(value.kind))) {
            return false;
        }
        if (!mapper.Map("range", value.range)) {
            return false;
        }
        if (!mapper.Map("selectionRange", value.selectionRange)) {
            return false;
        }
        return true;
    }

#pragma region Semantic Tokens

    //
    // Semantic Tokens
    //

    struct SemanticTokenClientsCapabilities
    {
        // /**
        //  * Whether implementation supports dynamic registration. If this is set to
        //  * `true` the client supports the new `(TextDocumentRegistrationOptions &
        //  * StaticRegistrationOptions)` return value for the corresponding server
        //  * capability as well.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * Which requests the client supports and might send to the server
        //  * depending on the server's capability. Please note that clients might not
        //  * show semantic tokens or degrade some of the user experience if a range
        //  * or full request is advertised by the client but not provided by the
        //  * server. If for example the client capability `requests.full` and
        //  * `request.range` are both set to true but the server only provides a
        //  * range provider the client might not render a minimap correctly or might
        //  * even decide to not show any semantic tokens at all.
        //  */
        // requests: { ... }
        struct
        {
            // /**
            //  * The client will send the `textDocument/semanticTokens/range` request
            //  * if the server provides a corresponding handler.
            //  */
            // range?: boolean | { };

            // /**
            //  * The client will send the `textDocument/semanticTokens/full` request
            //  * if the server provides a corresponding handler.
            //  */
            // full?: boolean | { delta?: boolean };
            std::optional<bool> full;

            // {
            // /**
            //  * The client will send the `textDocument/semanticTokens/full/delta`
            //  * request if the server provides a corresponding handler.
            //  */
            // delta?: boolean;
            std::optional<bool> delta;
            // }
        } requests;

        // /**
        //  * The token types that the client supports.
        //  */
        // 	tokenTypes: string[];

        // /**
        //  * The token modifiers that the client supports.
        //  */
        // 	tokenModifiers: string[];

        // /**
        //  * The formats the clients supports.
        //  */
        // 	formats: TokenFormat[];

        // /**
        //  * Whether the client supports tokens that can overlap each other.
        //  */
        // 	overlappingTokenSupport?: boolean;

        // /**
        //  * Whether the client supports tokens that can span multiple lines.
        //  */
        // 	multilineTokenSupport?: boolean;

        // /**
        //  * Whether the client allows the server to actively cancel a
        //  * semantic token request, e.g. supports returning
        //  * ErrorCodes.ServerCancelled. If a server does the client
        //  * needs to retrigger the request.
        //  *
        //  * @since 3.17.0
        //  */
        // 	serverCancelSupport?: boolean;

        // /**
        //  * Whether the client uses semantic tokens to augment existing
        //  * syntax tokens. If set to `true` client side created syntax
        //  * tokens and semantic tokens are both used for colorization. If
        //  * set to `false` the client only uses the returned semantic tokens
        //  * for colorization.
        //  *
        //  * If the value is `undefined` then the client behavior is not
        //  * specified.
        //  *
        //  * @since 3.17.0
        //  */
        // 	augmentsSyntaxTokens?: boolean;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, SemanticTokenClientsCapabilities& value) -> bool
    {
        {
            auto scopeGuard = mapper.EnterObjectScoped("requests");
            if (!mapper.Map("full", value.requests.full)) {
                return false;
            }

            if (!value.requests.full.has_value()) {
                auto scopeGuard = mapper.EnterObjectScoped("full");
                if (!mapper.Map("delta", value.requests.delta)) {
                    return false;
                }
            }
        }

        return true;
    }

    enum class SemanticTokenTypes
    {
        // The following are predefined values
        Namespace,
        Type,
        Class,
        Enum,
        Interface,
        Struct,
        TypeParameter,
        Variable,
        Property,
        EnumMember,
        Event,
        Function,
        Method,
        Macro,
        Keyword,
        Modifier,
        Comment,
        String,
        Number,
        Regexp,
        Operator,
        Decorator,

        //
    };

    enum class SemanticTokenModifiers
    {
    };

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

    struct SemanticTokenOptions /*: WorkDoneProgressOptions*/
    {
        // /**
        //  * The legend used by the server
        //  */
        // legend: SemanticTokensLegend;
        SemanticTokensLegend legend;

        // /**
        //  * Server supports providing semantic tokens for a specific range
        //  * of a document.
        //  */
        // range?: boolean | { };
        bool range;

        // /**
        //  * Server supports providing semantic tokens for a full document.
        //  */
        // full?: boolean | { delta?: boolean }
        bool full;

        // /**
        //  * The server supports deltas for full documents.
        //  */
        // delta?: boolean;
        bool delta;
    };

    struct SemanticTokensParam /*: WorkDoneProgressParams,PartialResultParams*/
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
        // resultId?: string;
        std::optional<std::string> resultId;

        // /**
        //  * The actual tokens.
        //  */
        // data: uinteger[];
        std::vector<uint32_t> data;
    };

    struct SemanticTokensEdit
    {
        // /**
        //  * The start offset of the edit.
        //  */
        // start: uinteger;
        uint32_t start;

        // /**
        //  * The count of elements to remove.
        //  */
        // deleteCount: uinteger;
        uint32_t deleteCount;

        // /**
        //  * The elements to insert.
        //  */
        // data?: uinteger[];
        std::vector<uint32_t> data;
    };

    struct SemanticTokenDelta
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

#pragma region Text Document Synchronization

    //
    // Text Document Synchronization
    //

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
    inline auto MapJson(JsonToObjectMapper& mapper, const TextDocumentSyncOptions& value) -> bool
    {
        if (!mapper.Map("openClose", value.openClose)) {
            return false;
        }
        if (!mapper.Map("change", static_cast<int32_t>(value.change))) {
            return false;
        }

        return true;
    }

    struct TextDocumentItem
    {
        // /**
        //  * The text document's URI.
        //  */
        // uri: DocumentUri;
        DocumentUri uri;

        // /**
        //  * The text document's language identifier.
        //  */
        // languageId: string;
        std::string languageId;

        // /**
        //  * The version number of this document (it will increase after each
        //  * change, including undo/redo).
        //  */
        // version: integer;
        int32_t version;

        // /**
        //  * The content of the opened text document.
        //  */
        // text: string;
        std::string text;
    };

    inline constexpr const char* LSPMethod_DidOpenTextDocument = "textDocument/didOpen";

    struct DidOpenTextDocumentParams
    {
        // /**
        //  * The document that was opened.
        //  */
        // textDocument: TextDocumentItem;
        TextDocumentItem textDocument;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, DidOpenTextDocumentParams& value) -> bool
    {
        {
            auto scopeGuard = mapper.EnterObjectScoped("textDocument");
            if (!mapper.Map("uri", value.textDocument.uri)) {
                return false;
            }
            if (!mapper.Map("languageId", value.textDocument.languageId)) {
                return false;
            }
            if (!mapper.Map("version", value.textDocument.version)) {
                return false;
            }
            if (!mapper.Map("text", value.textDocument.text)) {
                return false;
            }
        }

        return true;
    }

    struct TextDocumentContentChangeEvent
    {
        // /**
        //  * The range of the document that changed.
        //  */
        // range: Range;

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
    inline auto MapJson(JsonFromObjectMapper& mapper, TextDocumentContentChangeEvent& value) -> bool
    {
        if (!mapper.Map("text", value.text)) {
            return false;
        }

        return true;
    }

    inline constexpr const char* LSPMethod_DidChangeTextDocument = "textDocument/didChange";

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
    inline auto MapJson(JsonFromObjectMapper& mapper, DidChangeTextDocumentParams& value) -> bool
    {
        {
            auto scopeGuard = mapper.EnterObjectScoped("textDocument");
            if (!mapper.Map("uri", value.textDocument.uri)) {
                return false;
            }
            if (!mapper.Map("version", value.textDocument.version)) {
                return false;
            }
        }

        if (!mapper.Map("contentChanges", value.contentChanges)) {
            return false;
        }

        return true;
    }

    inline constexpr const char* LSPMethod_DidCloseTextDocument = "textDocument/didClose";

    struct DidCloseTextDocumentParams
    {
        // /**
        //  * The document that was closed.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, DidCloseTextDocumentParams& value) -> bool
    {
        {
            auto scopeGuard = mapper.EnterObjectScoped("textDocument");
            if (!mapper.Map("uri", value.textDocument)) {
                return false;
            }
        }

        return true;
    }

#pragma endregion

    //
    // Capabilities
    //

    struct TextDocumentClientCapabilities
    {
        // Unsupported capabilities omitted

        std::optional<SemanticTokenClientsCapabilities> semanticTokens;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, TextDocumentClientCapabilities& value) -> bool
    {
        if (!mapper.Map("semanticTokens", value.semanticTokens)) {
            return false;
        }

        return true;
    }

    struct ClientCapabilities
    {
        // Unsupported capabilities omitted

        // /**
        //  * Capabilities specific to the various semantic token requests.
        //  *
        //  * @since 3.16.0
        //  */
        // semanticTokens?: SemanticTokensClientCapabilities;
        std::optional<TextDocumentClientCapabilities> textDocument;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, ClientCapabilities& value) -> bool
    {
        if (!mapper.Map("textDocument", value.textDocument)) {
            return false;
        }

        return true;
    }

    struct ServerCapabilities
    {

        // /**
        //  * Defines how text documents are synced. Is either a detailed structure
        //  * defining each notification or for backwards compatibility the
        //  * TextDocumentSyncKind number. If omitted it defaults to
        //  * `TextDocumentSyncKind.None`.
        //  */
        // textDocumentSync?: TextDocumentSyncOptions | TextDocumentSyncKind;
        TextDocumentSyncOptions textDocumentSync;

        // /**
        //  * The server provides document symbol support.
        //  */
        // documentSymbolProvider ?: boolean | DocumentSymbolOptions;
        bool documentSymbolProvider;

        /**
         * The server provides semantic tokens support.
         *
         * @since 3.16.0
         */
        // semanticTokensProvider?: SemanticTokensOptions
        //     | SemanticTokensRegistrationOptions;
    };
    inline auto MapJson(JsonToObjectMapper& mapper, const ServerCapabilities& value) -> bool
    {
        if (!mapper.Map("textDocumentSync", value.textDocumentSync)) {
            return false;
        }
        if (!mapper.Map("documentSymbolProvider", value.documentSymbolProvider)) {
            return false;
        }

        return true;
    }

    //
    // Initialize
    //

    inline constexpr const char* LSPMethod_Initialize = "initialize";

    struct InitializeParams
    {
        // /**
        //  * The process Id of the parent process that started the server. Is null if
        //  * the process has not been started by another process. If the parent
        //  * process is not alive then the server should exit (see exit notification)
        //  * its process.
        //  */
        // processId: integer | null;
        std::optional<int32_t> processId;

        // /**
        //  * Information about the client
        //  *
        //  * @since 3.15.0
        //  */
        // clientInfo?: { ... }
        // struct
        // {
        // /**
        //  * The name of the client as defined by the client.
        //  */
        // name: string;

        // /**
        //  * The client's version as defined by the client.
        //  */
        // // version?: string;
        // } clientInfo;

        // /**
        //  * The locale the client is currently showing the user interface
        //  * in. This must not necessarily be the locale of the operating
        //  * system.
        //  *
        //  * Uses IETF language tags as the value's syntax
        //  * (See https://en.wikipedia.org/wiki/IETF_language_tag)
        //  *
        //  * @since 3.16.0
        //  */
        // locale?: string;

        // /**
        //  * The rootPath of the workspace. Is null
        //  * if no folder is open.
        //  *
        //  * @deprecated in favour of `rootUri`.
        //  */
        // rootPath?: string | null;

        // /**
        //  * The rootUri of the workspace. Is null if no
        //  * folder is open. If both `rootPath` and `rootUri` are set
        //  * `rootUri` wins.
        //  *
        //  * @deprecated in favour of `workspaceFolders`
        //  */
        // rootUri: DocumentUri | null;
        std::optional<DocumentUri> rootUri;

        // /**
        //  * User provided initialization options.
        //  */
        // initializationOptions?: LSPAny;

        // /**
        //  * The capabilities provided by the client (editor or tool)
        //  */
        //  capabilities: ClientCapabilities;
        ClientCapabilities capabilities;

        // /**
        //  * The initial trace setting. If omitted trace is disabled ('off').
        //  */
        // trace?: TraceValue;

        // /**
        //  * The workspace folders configured in the client when the server starts.
        //  * This property is only available if the client supports workspace folders.
        //  * It can be `null` if the client supports workspace folders but none are
        //  * configured.
        //  *
        //  * @since 3.6.0
        //  */
        // workspaceFolders?: WorkspaceFolder[] | null;
    };
    inline auto MapJson(JsonFromObjectMapper& mapper, InitializeParams& value) -> bool
    {
        if (!mapper.Map("processId", value.processId)) {
            return false;
        }
        if (!mapper.Map("rootUri", value.rootUri)) {
            return false;
        }
        if (!mapper.Map("capabilities", value.capabilities)) {
            return false;
        }

        return true;
    }

    struct InitializedResult
    {
        /**
         * The capabilities the language server provides.
         */
        // capabilities: ServerCapabilities;
        ServerCapabilities capabilities;

        /**
         * Information about the server.
         *
         * @since 3.15.0
         */
        // serverInfo?: { ... }
        struct
        {
            /**
             * The name of the server as defined by the server.
             */
            // name: string;
            std::string name;

            /**
             * The server's version as defined by the server.
             */
            // version?: string;
            std::string version;
        } serverInfo;
    };
    inline auto MapJson(JsonToObjectMapper& mapper, const InitializedResult& value) -> bool
    {
        if (!mapper.Map("capabilities", value.capabilities)) {
            return false;
        }

        {
            auto scopeGuard = mapper.EnterObjectScoped("serverInfo");

            if (!mapper.Map("name", value.serverInfo.name)) {
                return false;
            }
            if (!mapper.Map("version", value.serverInfo.version)) {
                return false;
            }
        }

        return true;
    }

    struct InitializeError
    {
        /**
         * Indicates whether the client execute the following retry logic:
         * (1) show the message provided by the ResponseError to the user
         * (2) user selects retry or cancel
         * (3) if user selected retry the initialize method is sent again.
         */
        // retry: boolean;
        bool retry;
    };
    inline auto MapJson(JsonToObjectMapper& mapper, const InitializeError& value) -> bool
    {
        if (!mapper.Map("retry", value.retry)) {
            return false;
        }

        return true;
    }

} // namespace glsld::lsp
