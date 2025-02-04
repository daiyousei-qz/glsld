#pragma once
#include "ObjectMapper.h"

#include <cstdint>
#include <string>
#include <optional>
#include <vector>

namespace glsld::lsp
{
    // using integer  = int32_t;
    // using uinteger = uint32_t;
    // using decimal  = float;

    using Uri         = std::string;
    using DocumentUri = Uri;

    struct TextDocumentIdentifier
    {
        // /**
        //  * The text document's URI.
        //  */
        // uri: DocumentUri;
        DocumentUri uri;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const TextDocumentIdentifier& value) -> bool
    {
        return mapper.Map("uri", value.uri);
    }
    inline auto MapJson(ObjectFromJsonMapper& mapper, TextDocumentIdentifier& value) -> bool
    {
        return mapper.Map("uri", value.uri);
    }

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
        // version: integer;
        int32_t version;
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
    inline auto MapJson(ObjectToJsonMapper& mapper, const Position& value) -> bool
    {
        return mapper.Map("line", value.line) && mapper.Map("character", value.character);
    }
    inline auto MapJson(ObjectFromJsonMapper& mapper, Position& value) -> bool
    {
        return mapper.Map("line", value.line) && mapper.Map("character", value.character);
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
    inline auto MapJson(ObjectToJsonMapper& mapper, const Range& value) -> bool
    {
        return mapper.Map("start", value.start) && mapper.Map("end", value.end);
    }
    inline auto MapJson(ObjectFromJsonMapper& mapper, Range& value) -> bool
    {
        return mapper.Map("start", value.start) && mapper.Map("end", value.end);
    }

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
    inline auto MapJson(ObjectToJsonMapper& mapper, const TextEdit& value) -> bool
    {
        return mapper.Map("range", value.range) && mapper.Map("newText", value.newText);
    }

    struct Location
    {
        // uri: DocumentUri;
        DocumentUri uri;

        // range: Range;
        Range range;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const Location& value) -> bool
    {
        return mapper.Map("uri", value.uri) && mapper.Map("range", value.range);
    }
    inline auto MapJson(ObjectFromJsonMapper& mapper, Location& value) -> bool
    {
        return mapper.Map("uri", value.uri) && mapper.Map("range", value.range);
    }

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

    // FIXME: should we use enum?
    // /**
    //  * Describes the content type that a client supports in various
    //  * result literals like `Hover`, `ParameterInfo` or `CompletionItem`.
    //  *
    //  * Please note that `MarkupKinds` must not start with a `$`. This kinds
    //  * are reserved for internal usage.
    //  */
    enum class MarkupKind
    {
        // /**
        //  * Plain text is supported as a content format
        //  */
        // export const PlainText: 'plaintext' = 'plaintext';
        PlainText,

        // /**
        //  * Markdown is supported as a content format
        //  */
        // export const Markdown: 'markdown' = 'markdown';
        Markdown,
    };

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
    //  *     kind: MarkupKind.Markdown,
    //  *     value: [
    //  *         '# Header',
    //  *         'Some text',
    //  *         '```typescript',
    //  *         'someCode();',
    //  *         '```'
    //  *     ].join('\n')
    //  * };
    //  * ```
    //  *
    //  * *Please Note* that clients might sanitize the return markdown. A client could
    //  * decide to remove HTML from the markdown to avoid script execution.
    //  */
    struct MarkupContent
    {
        // /**
        //  * The type of the Markup
        //  */
        // kind: MarkupKind;
        MarkupKind kind;

        // /**
        //  * The content itself
        //  */
        // value: string;
        std::string value;
    };

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
    inline auto MapJson(ObjectToJsonMapper& mapper, const ShowMessageParams& value) -> bool
    {
        return mapper.Map("type", static_cast<int32_t>(value.type)) && mapper.Map("message", value.message);
    }

#pragma endregion

#pragma region Go to Declaration

    //
    // Go to Declaration
    //

    inline constexpr const char* LSPMethod_Declaration = "textDocument/declaration";

    struct DeclarationClientCapabilities
    {
        // /**
        //  * Whether declaration supports dynamic registration. If this is set to
        //  * `true` the client supports the new `DeclarationRegistrationOptions`
        //  * return value for the corresponding server capability as well.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * The client supports additional metadata in the form of declaration links.
        //  */
        // linkSupport?: boolean;
    };

    struct DeclarationParams
    {
        TextDocumentPositionParams baseParams;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, DeclarationParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position);
    }

#pragma endregion

#pragma region Go to Definition

    //
    // Go to Definition
    //

    inline constexpr const char* LSPMethod_Definition = "textDocument/definition";

    struct DefinitionClientCapabilities
    {
        // /**
        //  * Whether definition supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * The client supports additional metadata in the form of definition links.
        //  *
        //  * @since 3.14.0
        //  */
        // linkSupport?: boolean;
    };

    struct DefinitionParams
    {
        TextDocumentPositionParams baseParams;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, DefinitionParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position);
    }

#pragma endregion

#pragma region Hover

    //
    // Hover
    //

    inline constexpr const char* LSPMethod_Hover = "textDocument/hover";

    struct HoverClientCapabilities
    {
        // /**
        //  * Whether hover supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * Client supports the follow content formats if the content
        //  * property refers to a `literal of type MarkupContent`.
        //  * The order describes the preferred format of the client.
        //  */
        // contentFormat?: MarkupKind[];
    };

    struct HoverOptions
    {
    };

    struct HoverParams
    {
        TextDocumentPositionParams baseParams;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, HoverParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position);
    }

    // /**
    //  * The result of a hover request.
    //  */
    struct Hover
    {
        // /**
        //  * The hover's content
        //  */
        // contents: MarkedString | MarkedString[] | MarkupContent;
        std::string contents;

        // /**
        //  * An optional range is a range inside a text document
        //  * that is used to visualize a hover, e.g. by changing the background color.
        //  */
        // range?: Range;
        Range range;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const Hover& value) -> bool
    {
        return mapper.Map("contents", value.contents) && mapper.Map("range", value.range);
    }

#pragma endregion

#pragma region Find References

    //
    // Find References
    //

    inline constexpr const char* LSPMethod_References = "textDocument/references";

    struct ReferenceClientCapabilities
    {
        // /**
        //  * Whether references supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;
    };

    struct ReferenceContext
    {
        // /**
        //  * Include the declaration of the current symbol.
        //  */
        // includeDeclaration: boolean;
        bool includeDeclaration;
    };

    struct ReferenceParams
    {
        TextDocumentPositionParams baseParams;

        // /**
        //  * Context carrying additional information.
        //  */
        // context: ReferenceContext;
        ReferenceContext context;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, ReferenceParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position) && mapper.MapObject("context", [&] {
                   return mapper.Map("includeDeclaration", value.context.includeDeclaration);
               });
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
        // /**
        //  * The symbol kind values the client supports. When this
        //  * property exists the client also guarantees that it will
        //  * handle values outside its set gracefully and falls back
        //  * to a default value when unknown.
        //  *
        //  * If this property is not present the client only supports
        //  * the symbol kinds from `File` to `Array` as defined in
        //  * the initial version of the protocol.
        //  */
        // valueSet?: SymbolKind[];
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
        //     /**
        //      * The tags supported by the client.
        //      */
        //     valueSet: SymbolTag[];
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
    inline auto MapJson(ObjectFromJsonMapper& mapper, DocumentSymbolParams& value) -> bool
    {
        return mapper.Map("textDocument", value.textDocument);
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
        std::vector<DocumentSymbol> children;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const DocumentSymbol& value) -> bool
    {
        return mapper.Map("name", value.name) && mapper.Map("kind", static_cast<int32_t>(value.kind)) &&
               mapper.Map("range", value.range) && mapper.Map("selectionRange", value.selectionRange) &&
               mapper.Map("children", value.children);
    }

#pragma endregion

#pragma region Semantic Tokens

    //
    // Semantic Tokens
    //

    inline constexpr const char* LSPMethod_SemanticTokensFull      = "textDocument/semanticTokens/full";
    inline constexpr const char* LSPMethod_SemanticTokensFullDelta = "textDocument/semanticTokens/full/delta";

    enum class SemanticTokenTypes
    {
        // namespace = 'namespace',
        // /**
        //  * Represents a generic type. Acts as a fallback for types which
        //  * can't be mapped to a specific type like class or enum.
        //  */
        // type = 'type',
        // class = 'class',
        // enum = 'enum',
        // interface = 'interface',
        // struct = 'struct',
        // typeParameter = 'typeParameter',
        // parameter = 'parameter',
        // variable = 'variable',
        // property = 'property',
        // enumMember = 'enumMember',
        // event = 'event',
        // function = 'function',
        // method = 'method',
        // macro = 'macro',
        // keyword = 'keyword',
        // modifier = 'modifier',
        // comment = 'comment',
        // string = 'string',
        // number = 'number',
        // regexp = 'regexp',
        // operator = 'operator'
        // /**
        //  * @since 3.17.0
        //  */
        // decorator = 'decorator'
    };

    enum class SemanticTokenModifiers
    {
        // declaration = 'declaration',
        // definition = 'definition',
        // readonly = 'readonly',
        // static = 'static',
        // deprecated = 'deprecated',
        // abstract = 'abstract',
        // async = 'async',
        // modification = 'modification',
        // documentation = 'documentation',
        // defaultLibrary = 'defaultLibrary'
    };

    enum class TokenFormat
    {
        // export const Relative: 'relative' = 'relative';
        Relative,
    };

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
        // tokenTypes: string[];

        // /**
        //  * The token modifiers that the client supports.
        //  */
        // tokenModifiers: string[];

        // /**
        //  * The formats the clients supports.
        //  */
        // formats: TokenFormat[];

        // /**
        //  * Whether the client supports tokens that can overlap each other.
        //  */
        // overlappingTokenSupport?: boolean;

        // /**
        //  * Whether the client supports tokens that can span multiple lines.
        //  */
        // multilineTokenSupport?: boolean;

        // /**
        //  * Whether the client allows the server to actively cancel a
        //  * semantic token request, e.g. supports returning
        //  * ErrorCodes.ServerCancelled. If a server does the client
        //  * needs to retrigger the request.
        //  *
        //  * @since 3.17.0
        //  */
        // serverCancelSupport?: boolean;

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
        // augmentsSyntaxTokens?: boolean;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, SemanticTokenClientsCapabilities& value) -> bool
    {
        return mapper.MapObject("requests", [&] {
            if (!mapper.Map("full", value.requests.full)) {
                return false;
            }

            if (!value.requests.full.has_value()) {
                if (!mapper.MapObject("full", [&] { return mapper.Map("delta", value.requests.delta); })) {
                    return false;
                }
            }

            return true;
        });
    }

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
    inline auto MapJson(ObjectToJsonMapper& mapper, const SemanticTokensLegend& value) -> bool
    {
        return mapper.Map("tokenTypes", value.tokenTypes) && mapper.Map("tokenModifiers", value.tokenModifiers);
    }

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

    template <typename Mapper>
    inline auto MapJson(ObjectFromJsonMapper& mapper, SemanticTokenOptions& value) -> bool
    {
        return mapper.Map("legend", value.legend) && mapper.Map("range", value.range) &&
               mapper.Map("full", value.full) && mapper.Map("delta", value.delta);
    }
    inline auto MapJson(ObjectToJsonMapper& mapper, const SemanticTokenOptions& value) -> bool
    {
        return mapper.Map("legend", value.legend) && mapper.Map("range", value.range) &&
               mapper.Map("full", value.full) && mapper.Map("delta", value.delta);
    }

    struct SemanticTokensParam /*: WorkDoneProgressParams,PartialResultParams*/
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, SemanticTokensParam& value) -> bool
    {
        return mapper.Map("textDocument", value.textDocument);
    }

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
    inline auto MapJson(ObjectToJsonMapper& mapper, const SemanticTokens& value) -> bool
    {
        return mapper.Map("resultId", value.resultId) && mapper.Map("data", value.data);
    }

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

    //
    // Inlay Hint
    //

    inline constexpr const char* LSPMethod_InlayHint = "textDocument/inlayHint";

    // /**
    //  * Inlay hint client capabilities.
    //  *
    //  * @since 3.17.0
    //  */
    struct InlayHintClientCapabilities
    {

        // /**
        //  * Whether inlay hints support dynamic registration.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * Indicates which properties a client can resolve lazily on a inlay
        //  * hint.
        //  */
        // resolveSupport ?: { }

        // /**
        //  * The properties that a client can resolve lazily.
        //  */
        // properties: string[];
    };

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
        // resolveProvider?: boolean;
        bool resolveProvider;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const InlayHintOptions& value) -> bool
    {
        return mapper.Map("resolveProvider", value.resolveProvider);
    }

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
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The visible document range for which inlay hints should be computed.
        //  */
        // range: Range;
        Range range;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, InlayHintParams& value) -> bool
    {
        return mapper.Map("textDocument", value.textDocument) && mapper.Map("range", value.range);
    }

    // /**
    //  * Inlay hint information.
    //  *
    //  * @since 3.17.0
    //  */
    struct InlayHint
    {
        // /**
        //  * The position of this hint.
        //  */
        // position: Position;
        Position position;

        // /**
        //  * The label of this hint. A human readable string or an array of
        //  * InlayHintLabelPart label parts.
        //  *
        //  * *Note* that neither the string nor the label part can be empty.
        //  */
        // label: string | InlayHintLabelPart[];
        std::string label;

        // /**
        //  * The kind of this hint. Can be omitted in which case the client
        //  * should fall back to a reasonable default.
        //  */
        // kind?: InlayHintKind;

        // /**
        //  * Optional text edits that are performed when accepting this inlay hint.
        //  *
        //  * *Note* that edits are expected to change the document so that the inlay
        //  * hint (or its nearest variant) is now part of the document and the inlay
        //  * hint itself is now obsolete.
        //  *
        //  * Depending on the client capability `inlayHint.resolveSupport` clients
        //  * might resolve this property late using the resolve request.
        //  */
        // textEdits?: TextEdit[];

        // /**
        //  * The tooltip text when you hover over this item.
        //  *
        //  * Depending on the client capability `inlayHint.resolveSupport` clients
        //  * might resolve this property late using the resolve request.
        //  */
        // tooltip?: string | MarkupContent;

        // /**
        //  * Render padding before the hint.
        //  *
        //  * Note: Padding should use the editor's background color, not the
        //  * background color of the hint itself. That means padding can be used
        //  * to visually align/separate an inlay hint.
        //  */
        // paddingLeft?: boolean;
        bool paddingLeft;

        // /**
        //  * Render padding after the hint.
        //  *
        //  * Note: Padding should use the editor's background color, not the
        //  * background color of the hint itself. That means padding can be used
        //  * to visually align/separate an inlay hint.
        //  */
        // paddingRight?: boolean;
        bool paddingRight;

        // /**
        //  * A data entry field that is preserved on a inlay hint between
        //  * a `textDocument/inlayHint` and a `inlayHint/resolve` request.
        //  */
        // data?: LSPAny;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const InlayHint& value) -> bool
    {
        return mapper.Map("position", value.position) && mapper.Map("label", value.label) &&
               mapper.Map("paddingLeft", value.paddingLeft) && mapper.Map("paddingRight", value.paddingRight);
    }

#pragma endregion

#pragma region Completion Proposal

    //
    // Completion Proposal
    //

    inline constexpr const char* LSPMethod_Completion = "textDocument/completion";

    struct CompletionClientCapabilities
    {
        // /**
        //  * Whether completion supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * The client supports the following `CompletionItem` specific
        //  * capabilities.
        //  */
        // completionItem?: { ... }
        struct
        {
            // /**
            //  * Client supports snippets as insert text.
            //  *
            //  * A snippet can define tab stops and placeholders with `$1`, `$2`
            //  * and `${3:foo}`. `$0` defines the final tab stop, it defaults to
            //  * the end of the snippet. Placeholders with equal identifiers are
            //  * linked, that is typing in one will update others too.
            //  */
            // snippetSupport?: boolean;
            bool snippetSupport;

            // /**
            //  * Client supports commit characters on a completion item.
            //  */
            // commitCharactersSupport?: boolean;
            bool commitCharactersSupport;

            // /**
            //  * Client supports the follow content formats for the documentation
            //  * property. The order describes the preferred format of the client.
            //  */
            // documentationFormat?: MarkupKind[];

            // /**
            //  * Client supports the deprecated property on a completion item.
            //  */
            // deprecatedSupport?: boolean;
            bool deprecatedSupport;

            // /**
            //  * Client supports the preselect property on a completion item.
            //  */
            // preselectSupport?: boolean;
            bool preselectSupport;

            // /**
            //  * Client supports the tag property on a completion item. Clients
            //  * supporting tags have to handle unknown tags gracefully. Clients
            //  * especially need to preserve unknown tags when sending a completion
            //  * item back to the server in a resolve call.
            //  *
            //  * @since 3.15.0
            //  */
            // tagSupport?: {
            // /**
            //  * The tags supported by the client.
            //  */
            // valueSet: CompletionItemTag[];

            // /**
            //  * Client supports insert replace edit to control different behavior if
            //  * a completion item is inserted in the text or should replace text.
            //  *
            //  * @since 3.16.0
            //  */
            // insertReplaceSupport?: boolean;
            bool insertReplaceSupport;

            // /**
            //  * Indicates which properties a client can resolve lazily on a
            //  * completion item. Before version 3.16.0 only the predefined properties
            //  * `documentation` and `detail` could be resolved lazily.
            //  *
            //  * @since 3.16.0
            //  */
            // resolveSupport?: {
            //     /**
            //      * The properties that a client can resolve lazily.
            //      */
            //     properties: string[];
            // };

            // /**
            //  * The client supports the `insertTextMode` property on
            //  * a completion item to override the whitespace handling mode
            //  * as defined by the client (see `insertTextMode`).
            //  *
            //  * @since 3.16.0
            //  */
            // insertTextModeSupport?: {
            //     valueSet: InsertTextMode[];
            // };

            // /**
            //  * The client has support for completion item label
            //  * details (see also `CompletionItemLabelDetails`).
            //  *
            //  * @since 3.17.0
            //  */
            // labelDetailsSupport?: boolean;
        } completionItem;

        // completionItemKind?: {
        //     /**
        //      * The completion item kind values the client supports. When this
        //      * property exists the client also guarantees that it will
        //      * handle values outside its set gracefully and falls back
        //      * to a default value when unknown.
        //      *
        //      * If this property is not present the client only supports
        //      * the completion items kinds from `Text` to `Reference` as defined in
        //      * the initial version of the protocol.
        //      */
        //     valueSet?: CompletionItemKind[];
        // };

        // /**
        //  * The client supports to send additional context information for a
        //  * `textDocument/completion` request.
        //  */
        // contextSupport?: boolean;

        // /**
        //  * The client's default when the completion item doesn't provide a
        //  * `insertTextMode` property.
        //  *
        //  * @since 3.17.0
        //  */
        // insertTextMode?: InsertTextMode;

        // /**
        //  * The client supports the following `CompletionList` specific
        //  * capabilities.
        //  *
        //  * @since 3.17.0
        //  */
        // completionList?: {
        //     /**
        //      * The client supports the following itemDefaults on
        //      * a completion list.
        //      *
        //      * The value lists the supported property names of the
        //      * `CompletionList.itemDefaults` object. If omitted
        //      * no properties are supported.
        //      *
        //      * @since 3.17.0
        //      */
        //     itemDefaults?: string[];
        // }
    };

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
        // triggerCharacters?: string[];
        std::vector<std::string> triggerCharacters;

        // /**
        //  * The list of all possible characters that commit a completion. This field
        //  * can be used if clients don't support individual commit characters per
        //  * completion item. See client capability
        //  * `completion.completionItem.commitCharactersSupport`.
        //  *
        //  * If a server provides both `allCommitCharacters` and commit characters on
        //  * an individual completion item the ones on the completion item win.
        //  *
        //  * @since 3.2.0
        //  */
        // allCommitCharacters?: string[];
        std::vector<std::string> allCommitCharacters;

        // /**
        //  * The server provides support to resolve additional
        //  * information for a completion item.
        //  */
        // resolveProvider?: boolean;
        bool resolveProvider;

        // /**
        //  * The server supports the following `CompletionItem` specific
        //  * capabilities.
        //  *
        //  * @since 3.17.0
        //  */
        // completionItem?: {
        //     /**
        //      * The server has support for completion item label
        //      * details (see also `CompletionItemLabelDetails`) when receiving
        //      * a completion item in a resolve call.
        //      *
        //      * @since 3.17.0
        //      */
        //     labelDetailsSupport?: boolean;
        // }
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const CompletionOptions& value) -> bool
    {
        return mapper.Map("triggerCharacters", value.triggerCharacters) &&
               mapper.Map("allCommitCharacters", value.allCommitCharacters) &&
               mapper.Map("resolveProvider", value.resolveProvider);
    }

    // /**
    //  * How a completion was triggered
    //  */
    enum class CompletionTriggerKind
    {
        // /**
        //  * Completion was triggered by typing an identifier (24x7 code
        //  * complete), manual invocation (e.g Ctrl+Space) or via API.
        //  */
        // export const Invoked: 1 = 1;
        Invoked = 1,

        // /**
        //  * Completion was triggered by a trigger character specified by
        //  * the `triggerCharacters` properties of the
        //  * `CompletionRegistrationOptions`.
        //  */
        // export const TriggerCharacter: 2 = 2;
        TriggerCharacter = 2,

        // /**
        //  * Completion was re-triggered as the current completion list is incomplete.
        //  */
        // export const TriggerForIncompleteCompletions: 3 = 3;
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
        // triggerKind: CompletionTriggerKind;
        CompletionTriggerKind triggerKind;

        // /**
        //  * The trigger character (a single character) that has trigger code
        //  * complete. Is undefined if
        //  * `triggerKind !== CompletionTriggerKind.TriggerCharacter`
        //  */
        // triggerCharacter?: string;
        std::optional<std::string> triggerCharacter;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, CompletionContext& value) -> bool
    {
        // FIXME: fix object map
        uint32_t tmpTriggerKind;
        if (!mapper.Map("triggerKind", tmpTriggerKind)) {
            value.triggerKind = static_cast<CompletionTriggerKind>(tmpTriggerKind);
            return false;
        }
        if (!mapper.Map("triggerCharacter", value.triggerCharacter)) {
            return false;
        }

        return true;
    }

    struct CompletionParams
    {
        TextDocumentPositionParams baseParams;

        //  * The completion context. This is only available if the client specifies
        // /**
        //  * to send this using the client capability
        //  * `completion.contextSupport === true`
        //  */
        // context?: CompletionContext;
        // std::optional<CompletionContext> context;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, CompletionParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position);
    }

    // /**
    //  * Defines whether the insert text in a completion item should be interpreted as
    //  * plain text or a snippet.
    //  */
    enum class InsertTextFormat
    {
        // /**
        //  * The primary text to be inserted is treated as a plain string.
        //  */
        // export const PlainText = 1;
        PlainText = 1,

        // /**
        //  * The primary text to be inserted is treated as a snippet.
        //  *
        //  * A snippet can define tab stops and placeholders with `$1`, `$2`
        //  * and `${3:foo}`. `$0` defines the final tab stop, it defaults to
        //  * the end of the snippet. Placeholders with equal identifiers are linked,
        //  * that is typing in one will update others too.
        //  */
        // export const Snippet = 2;
        Snippet = 2,
    };

    // /**
    //  * Completion item tags are extra annotations that tweak the rendering of a
    //  * completion item.
    //  *
    //  * @since 3.15.0
    //  */
    enum class CompletionItemTag
    {
        // /**
        //  * Render a completion as obsolete, usually using a strike-out.
        //  */
        // export const Deprecated = 1;
        Deprecated = 1,
    };

    // /**
    //  * A special text edit to provide an insert and a replace operation.
    //  *
    //  * @since 3.16.0
    //  */
    struct InsertReplaceEdit
    {
        // /**
        //  * The string to be inserted.
        //  */
        // newText: string;
        std::string newText;

        // /**
        //  * The range if the insert is requested
        //  */
        // insert: Range;
        Range insert;

        // /**
        //  * The range if the replace is requested.
        //  */
        // replace: Range;
        Range replace;
    };

    // /**
    //  * How whitespace and indentation is handled during completion
    //  * item insertion.
    //  *
    //  * @since 3.16.0
    //  */
    enum class InsertTextMode
    {
        // /**
        //  * The insertion or replace strings is taken as it is. If the
        //  * value is multi line the lines below the cursor will be
        //  * inserted using the indentation defined in the string value.
        //  * The client will not apply any kind of adjustments to the
        //  * string.
        //  */
        // export const asIs: 1 = 1;
        AsIs = 1,

        // /**
        //  * The editor adjusts leading whitespace of new lines so that
        //  * they match the indentation up to the cursor of the line for
        //  * which the item is accepted.
        //  *
        //  * Consider a line like this: <2tabs><cursor><3tabs>foo. Accepting a
        //  * multi line completion item is indented using 2 tabs and all
        //  * following lines inserted will be indented using 2 tabs as well.
        //  */
        // export const adjustIndentation: 2 = 2;
        AdjustIndentation = 2,
    };

    // /**
    //  * Additional details for a completion item label.
    //  *
    //  * @since 3.17.0
    //  */
    struct CompletionItemLabelDetails
    {
        // /**
        //  * An optional string which is rendered less prominently directly after
        //  * {@link CompletionItem.label label}, without any spacing. Should be
        //  * used for function signatures or type annotations.
        //  */
        // detail?: string;
        std::string detail;

        // /**
        //  * An optional string which is rendered less prominently after
        //  * {@link CompletionItemLabelDetails.detail}. Should be used for fully qualified
        //  * names or file path.
        //  */
        // description?: string;
        std::string description;
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
        // label: string;
        std::string label;

        // /**
        //  * Additional details for the label
        //  *
        //  * @since 3.17.0
        //  */
        // labelDetails?: CompletionItemLabelDetails;

        // /**
        //  * The kind of this completion item. Based of the kind
        //  * an icon is chosen by the editor. The standardized set
        //  * of available values is defined in `CompletionItemKind`.
        //  */
        // kind?: CompletionItemKind;
        CompletionItemKind kind;

        // /**
        //  * Tags for this completion item.
        //  *
        //  * @since 3.15.0
        //  */
        // tags?: CompletionItemTag[];

        // /**
        //  * A human-readable string with additional information
        //  * about this item, like type or symbol information.
        //  */
        // detail?: string;

        // /**
        //  * A human-readable string that represents a doc-comment.
        //  */
        // documentation?: string | MarkupContent;

        // /**
        //  * Indicates if this item is deprecated.
        //  *
        //  * @deprecated Use `tags` instead if supported.
        //  */
        // deprecated?: boolean;

        // /**
        //  * Select this item when showing.
        //  *
        //  * *Note* that only one completion item can be selected and that the
        //  * tool / client decides which item that is. The rule is that the *first*
        //  * item of those that match best is selected.
        //  */
        // preselect?: boolean;

        // /**
        //  * A string that should be used when comparing this item
        //  * with other items. When `falsy` the label is used
        //  * as the sort text for this item.
        //  */
        // sortText?: string;

        // /**
        //  * A string that should be used when filtering a set of
        //  * completion items. When `falsy` the label is used as the
        //  * filter text for this item.
        //  */
        // filterText?: string;

        // /**
        //  * A string that should be inserted into a document when selecting
        //  * this completion. When `falsy` the label is used as the insert text
        //  * for this item.
        //  *
        //  * The `insertText` is subject to interpretation by the client side.
        //  * Some tools might not take the string literally. For example
        //  * VS Code when code complete is requested in this example
        //  * `con<cursor position>` and a completion item with an `insertText` of
        //  * `console` is provided it will only insert `sole`. Therefore it is
        //  * recommended to use `textEdit` instead since it avoids additional client
        //  * side interpretation.
        //  */
        // insertText?: string;

        // /**
        //  * The format of the insert text. The format applies to both the
        //  * `insertText` property and the `newText` property of a provided
        //  * `textEdit`. If omitted defaults to `InsertTextFormat.PlainText`.
        //  *
        //  * Please note that the insertTextFormat doesn't apply to
        //  * `additionalTextEdits`.
        //  */
        // insertTextFormat?: InsertTextFormat;

        // /**
        //  * How whitespace and indentation is handled during completion
        //  * item insertion. If not provided the client's default value depends on
        //  * the `textDocument.completion.insertTextMode` client capability.
        //  *
        //  * @since 3.16.0
        //  * @since 3.17.0 - support for `textDocument.completion.insertTextMode`
        //  */
        // insertTextMode?: InsertTextMode;

        // /**
        //  * An edit which is applied to a document when selecting this completion.
        //  * When an edit is provided the value of `insertText` is ignored.
        //  *
        //  * *Note:* The range of the edit must be a single line range and it must
        //  * contain the position at which completion has been requested.
        //  *
        //  * Most editors support two different operations when accepting a completion
        //  * item. One is to insert a completion text and the other is to replace an
        //  * existing text with a completion text. Since this can usually not be
        //  * predetermined by a server it can report both ranges. Clients need to
        //  * signal support for `InsertReplaceEdit`s via the
        //  * `textDocument.completion.completionItem.insertReplaceSupport` client
        //  * capability property.
        //  *
        //  * *Note 1:* The text edit's range as well as both ranges from an insert
        //  * replace edit must be a [single line] and they must contain the position
        //  * at which completion has been requested.
        //  * *Note 2:* If an `InsertReplaceEdit` is returned the edit's insert range
        //  * must be a prefix of the edit's replace range, that means it must be
        //  * contained and starting at the same position.
        //  *
        //  * @since 3.16.0 additional type `InsertReplaceEdit`
        //  */
        // textEdit?: TextEdit | InsertReplaceEdit;

        // /**
        //  * The edit text used if the completion item is part of a CompletionList and
        //  * CompletionList defines an item default for the text edit range.
        //  *
        //  * Clients will only honor this property if they opt into completion list
        //  * item defaults using the capability `completionList.itemDefaults`.
        //  *
        //  * If not provided and a list's default range is provided the label
        //  * property is used as a text.
        //  *
        //  * @since 3.17.0
        //  */
        // textEditText?: string;

        // /**
        //  * An optional array of additional text edits that are applied when
        //  * selecting this completion. Edits must not overlap (including the same
        //  * insert position) with the main edit nor with themselves.
        //  *
        //  * Additional text edits should be used to change text unrelated to the
        //  * current cursor position (for example adding an import statement at the
        //  * top of the file if the completion item will insert an unqualified type).
        //  */
        // additionalTextEdits?: TextEdit[];

        // /**
        //  * An optional set of characters that when pressed while this completion is
        //  * active will accept it first and then type that character. *Note* that all
        //  * commit characters should have `length=1` and that superfluous characters
        //  * will be ignored.
        //  */
        // commitCharacters?: string[];

        // /**
        //  * An optional command that is executed *after* inserting this completion.
        //  * *Note* that additional modifications to the current document should be
        //  * described with the additionalTextEdits-property.
        //  */
        // command?: Command;

        // /**
        //  * A data entry field that is preserved on a completion item between
        //  * a completion and a completion resolve request.
        //  */
        // data?: LSPAny;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const CompletionItem& value) -> bool
    {
        return mapper.Map("label", value.label) && mapper.Map("kind", static_cast<int32_t>(value.kind));
    }

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
        // itemDefaults?: {
        // /**
        //  * A default commit character set.
        //  *
        //  * @since 3.17.0
        //  */
        // commitCharacters?: string[];

        // /**
        //  * A default edit range
        //  *
        //  * @since 3.17.0
        //  */
        // editRange?: Range | {
        //     insert: Range;
        //     replace: Range;
        // };

        // /**
        //  * A default insert text format
        //  *
        //  * @since 3.17.0
        //  */
        // insertTextFormat?: InsertTextFormat;

        // /**
        //  * A default insert text mode
        //  *
        //  * @since 3.17.0
        //  */
        // insertTextMode?: InsertTextMode;

        // /**
        //  * A default data value.
        //  *
        //  * @since 3.17.0
        //  */
        // data?: LSPAny;
        // }

        // /**
        //  * The completion items.
        //  */
        // items: CompletionItem[];
        std::vector<CompletionItem> items;
    };

#pragma endregion

#pragma region Signature Help

    //
    // Signature Help
    //
    inline constexpr const char* LSPMethod_SignatureHelp = "textDocument/signatureHelp";

    struct SignatureHelpClientCapabilities
    {
        // /**
        //  * Whether signature help supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;

        // /**
        //  * The client supports the following `SignatureInformation`
        //  * specific properties.
        //  */
        // signatureInformation?: {
        // 	/**
        // 	 * Client supports the follow content formats for the documentation
        // 	 * property. The order describes the preferred format of the client.
        // 	 */
        // 	documentationFormat?: MarkupKind[];

        // 	/**
        // 	 * Client capabilities specific to parameter information.
        // 	 */
        // 	parameterInformation?: {
        // 		/**
        // 		 * The client supports processing label offsets instead of a
        // 		 * simple label string.
        // 		 *
        // 		 * @since 3.14.0
        // 		 */
        // 		labelOffsetSupport?: boolean;
        // 	};

        // /**
        //  * The client supports the `activeParameter` property on
        //  * `SignatureInformation` literal.
        //  *
        //  * @since 3.16.0
        //  */
        // 	activeParameterSupport?: boolean;
        // };

        // /**
        //  * The client supports to send additional context information for a
        //  * `textDocument/signatureHelp` request. A client that opts into
        //  * contextSupport will also support the `retriggerCharacters` on
        //  * `SignatureHelpOptions`.
        //  *
        //  * @since 3.15.0
        //  */
        // contextSupport?: boolean;
    };

    struct SignatureHelpOptions
    {
        // /**
        //  * The characters that trigger signature help
        //  * automatically.
        //  */
        // triggerCharacters?: string[];
        std::vector<std::string> triggerCharacters;

        // /**
        //  * List of characters that re-trigger signature help.
        //  *
        //  * These trigger characters are only active when signature help is already
        //  * showing. All trigger characters are also counted as re-trigger
        //  * characters.
        //  *
        //  * @since 3.15.0
        //  */
        // retriggerCharacters?: string[];
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const SignatureHelpOptions& value) -> bool
    {
        return mapper.Map("triggerCharacters", value.triggerCharacters);
    }

    enum SignatureHelpTriggerKind
    {
        // /**
        //  * Signature help was invoked manually by the user or by a command.
        //  */
        // export const Invoked: 1 = 1;
        Invoke = 1,
        // /**
        //  * Signature help was triggered by a trigger character.
        //  */
        // export const TriggerCharacter: 2 = 2;
        TriggerCharacter = 2,
        // /**
        //  * Signature help was triggered by the cursor moving or by the document
        //  * content changing.
        //  */
        // export const ContentChange: 3 = 3;
        ContentChange = 3,
    };

    // /**
    //  * Additional information about the context in which a signature help request
    //  * was triggered.
    //  *
    //  * @since 3.15.0
    //  */
    struct SignatureHelpContext
    {
        // /**
        //  * Action that caused signature help to be triggered.
        //  */
        // triggerKind: SignatureHelpTriggerKind;

        // /**
        //  * Character that caused signature help to be triggered.
        //  *
        //  * This is undefined when triggerKind !==
        //  * SignatureHelpTriggerKind.TriggerCharacter
        //  */
        // triggerCharacter?: string;

        // /**
        //  * `true` if signature help was already showing when it was triggered.
        //  *
        //  * Retriggers occur when the signature help is already active and can be
        //  * caused by actions such as typing a trigger character, a cursor move, or
        //  * document content changes.
        //  */
        // isRetrigger: boolean;

        // /**
        //  * The currently active `SignatureHelp`.
        //  *
        //  * The `activeSignatureHelp` has its `SignatureHelp.activeSignature` field
        //  * updated based on the user navigating through available signatures.
        //  */
        // activeSignatureHelp?: SignatureHelp;
    };

    struct SignatureHelpParams
    {
        TextDocumentPositionParams baseParams;

        // /**
        //  * The signature help context. This is only available if the client
        //  * specifies to send this using the client capability
        //  * `textDocument.signatureHelp.contextSupport === true`
        //  *
        //  * @since 3.15.0
        //  */
        // context?: SignatureHelpContext;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, SignatureHelpParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position);
    }

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
        // label: string | [uinteger, uinteger];

        // /**
        //  * The human-readable doc-comment of this parameter. Will be shown
        //  * in the UI but can be omitted.
        //  */
        // documentation?: string | MarkupContent;
    };
    // inline auto MapJson(ObjectToJsonMapper& mapper, const ParameterInformation& value) -> bool
    // {
    //     return true;
    // }

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

        // /**
        //  * The index of the active parameter.
        //  *
        //  * If provided, this is used in place of `SignatureHelp.activeParameter`.
        //  *
        //  * @since 3.16.0
        //  */
        // activeParameter?: uinteger;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const SignatureInformation& value) -> bool
    {
        if (!mapper.Map("label", value.label)) {
            return false;
        }

        {
            // FIXME: implement MarkupContent properly
            // if (!mapper.Map("documentation", value.documentation)) {
            //     return false;
            // }

            auto scopeGuard  = mapper.EnterObjectScoped("documentation");
            std::string kind = "markdown";
            if (!mapper.Map("kind", kind)) {
                return false;
            }
            if (!mapper.Map("value", value.documentation)) {
                return false;
            }
        }

        return true;
    }

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
        // signatures: SignatureInformation[];
        std::vector<SignatureInformation> signatures;

        // /**
        //  * The active signature. If omitted or the value lies outside the
        //  * range of `signatures` the value defaults to zero or is ignore if
        //  * the `SignatureHelp` as no signatures.
        //  *
        //  * Whenever possible implementors should make an active decision about
        //  * the active signature and shouldn't rely on a default value.
        //  *
        //  * In future version of the protocol this property might become
        //  * mandatory to better express this.
        //  */
        // activeSignature?: uinteger;

        // /**
        //  * The active parameter of the active signature. If omitted or the value
        //  * lies outside the range of `signatures[activeSignature].parameters`
        //  * defaults to 0 if the active signature has parameters. If
        //  * the active signature has no parameters it is ignored.
        //  * In future version of the protocol this property might become
        //  * mandatory to better express the active parameter if the
        //  * active signature does have any.
        //  */
        // activeParameter?: uinteger;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const SignatureHelp& value) -> bool
    {
        return mapper.Map("signatures", value.signatures);
    }

#pragma endregion

#pragma region Document Color

    //
    // Document Color
    //
    inline constexpr const char* LSPMethod_DocumentColor     = "textDocument/documentColor";
    inline constexpr const char* LSPMethod_ColorPresentation = "textDocument/colorPresentation";

    struct DocumentColorClientCapabilities
    {
        // /**
        //  * Whether document color supports dynamic registration.
        //  */
        // dynamicRegistration?: boolean;
    };

    struct DocumentColorOptions
    {
    };

    struct DocumentColorParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, DocumentColorParams& value) -> bool
    {
        return mapper.Map("textDocument", value.textDocument);
    }

    // /**
    //  * Represents a color in RGBA space.
    //  */
    struct Color
    {
        // /**
        //  * The red component of this color in the range [0-1].
        //  */
        // readonly red: decimal;
        double red;

        // /**
        //  * The green component of this color in the range [0-1].
        //  */
        // readonly green: decimal;
        double green;

        // /**
        //  * The blue component of this color in the range [0-1].
        //  */
        // readonly blue: decimal;
        double blue;

        // /**
        //  * The alpha component of this color in the range [0-1].
        //  */
        // readonly alpha: decimal;
        double alpha;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const Color& value) -> bool
    {
        return mapper.Map("red", value.red) && mapper.Map("green", value.green) && mapper.Map("blue", value.blue) &&
               mapper.Map("alpha", value.alpha);
    }
    inline auto MapJson(ObjectFromJsonMapper& mapper, Color& value) -> bool
    {
        return mapper.Map("red", value.red) && mapper.Map("green", value.green) && mapper.Map("blue", value.blue) &&
               mapper.Map("alpha", value.alpha);
    }

    struct ColorInformation
    {
        // /**
        //  * The range in the document where this color appears.
        //  */
        // range: Range;
        Range range;

        // /**
        //  * The actual color value for this color range.
        //  */
        // color: Color;
        Color color;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const ColorInformation& value) -> bool
    {
        return mapper.Map("range", value.range) && mapper.Map("color", value.color);
    }

    struct ColorPresentationParams
    {
        // /**
        //  * The text document.
        //  */
        // textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        // /**
        //  * The color information to request presentations for.
        //  */
        // color: Color;
        Color color;

        // /**
        //  * The range where the color would be inserted. Serves as a context.
        //  */
        // range: Range;
        Range range;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, ColorPresentationParams& value) -> bool
    {
        return mapper.Map("textDocument", value.textDocument) && mapper.Map("color", value.color) &&
               mapper.Map("range", value.range);
    }

    struct ColorPresentation
    {
        // /**
        //  * The label of this color presentation. It will be shown on the color
        //  * picker header. By default this is also the text that is inserted when
        //  * selecting this color presentation.
        //  */
        // label: string;
        std::string label;

        // /**
        //  * An [edit](#TextEdit) which is applied to a document when selecting
        //  * this presentation for the color. When `falsy` the
        //  * [label](#ColorPresentation.label) is used.
        //  */
        // textEdit?: TextEdit;
        std::optional<TextEdit> textEdit;

        // /**
        //  * An optional array of additional [text edits](#TextEdit) that are applied
        //  * when selecting this color presentation. Edits must not overlap with the
        //  * main [edit](#ColorPresentation.textEdit) nor with themselves.
        //  */
        // additionalTextEdits?: TextEdit[];
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const ColorPresentation& value) -> bool
    {
        return mapper.Map("label", value.label) && mapper.Map("textEdit", value.textEdit);
    }

#pragma endregion

#pragma region Rename

    //
    // Rename
    //

    inline constexpr const char* LSPMethod_Rename        = "textDocument/rename";
    inline constexpr const char* LSPMethod_PrepareRename = "textDocument/prepareRename";

    struct RenameOptions
    {
        // /**
        //  * Renames should be checked and tested before being executed.
        //  */
        // prepareProvider?: boolean;
        bool prepareProvider;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const RenameOptions& value) -> bool
    {
        return mapper.Map("prepareProvider", value.prepareProvider);
    }

    struct RenameParams
    {
        TextDocumentPositionParams baseParams;

        // /**
        //  * The new name of the symbol. If the given name is not valid the
        //  * request must return a [ResponseError](#ResponseError) with an
        //  * appropriate message set.
        //  */
        // newName: string;
        std::string newName;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, RenameParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position) && mapper.Map("newName", value.newName);
    }

    struct PrepareRenameParams
    {
        TextDocumentPositionParams baseParams;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, PrepareRenameParams& value) -> bool
    {
        return mapper.Map("textDocument", value.baseParams.textDocument) &&
               mapper.Map("position", value.baseParams.position);
    }

#pragma endregion

#pragma region Text Document Synchronization

    //
    // Text Document Synchronization
    //

    struct TextDocumentClientCapabilities
    {
        // Unsupported capabilities omitted

        std::optional<SemanticTokenClientsCapabilities> semanticTokens;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, TextDocumentClientCapabilities& value) -> bool
    {
        return mapper.Map("semanticTokens", value.semanticTokens);
    }

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
    inline auto MapJson(ObjectToJsonMapper& mapper, const TextDocumentSyncOptions& value) -> bool
    {
        return mapper.Map("openClose", value.openClose) && mapper.Map("change", static_cast<int32_t>(value.change));
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
    inline auto MapJson(ObjectFromJsonMapper& mapper, DidOpenTextDocumentParams& value) -> bool
    {
        return mapper.MapObject("textDocument", [&] {
            return mapper.Map("uri", value.textDocument.uri) &&
                   mapper.Map("languageId", value.textDocument.languageId) &&
                   mapper.Map("version", value.textDocument.version) && mapper.Map("text", value.textDocument.text);
        });
    }

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
    inline auto MapJson(ObjectFromJsonMapper& mapper, TextDocumentContentChangeEvent& value) -> bool
    {
        return mapper.Map("range", value.range) && mapper.Map("text", value.text);
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
    inline auto MapJson(ObjectFromJsonMapper& mapper, DidChangeTextDocumentParams& value) -> bool
    {
        return mapper.MapObject("textDocument", [&] {
            return mapper.Map("uri", value.textDocument.uri) && mapper.Map("version", value.textDocument.version);
        }) && mapper.Map("contentChanges", value.contentChanges);
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
    inline auto MapJson(ObjectFromJsonMapper& mapper, DidCloseTextDocumentParams& value) -> bool
    {
        return mapper.Map("textDocument", value.textDocument);
    }

#pragma endregion

    //
    // Capabilities
    //

    // This class doesn't strictly match the LSP spec.
    struct ClientCapabilities
    {
        // /**
        //  * The position encodings supported by the client. Client and server
        //  * have to agree on the same position encoding to ensure that offsets
        //  * (e.g. character position in a line) are interpreted the same on both
        //  * side.
        //  *
        //  * To keep the protocol backwards compatible the following applies: if
        //  * the value 'utf-16' is missing from the array of position encodings
        //  * servers can assume that the client supports UTF-16. UTF-16 is
        //  * therefore a mandatory encoding.
        //  *
        //  * If omitted it defaults to ['utf-16'].
        //  *
        //  * Implementation considerations: since the conversion from one encoding
        //  * into another requires the content of the file / line the conversion
        //  * is best done where the file is read which is usually on the server
        //  * side.
        //  *
        //  * @since 3.17.0
        //  */
        // positionEncodings?: PositionEncodingKind[];
        std::vector<std::string> positionEncodings;
    };
    inline auto MapJson(ObjectFromJsonMapper& mapper, ClientCapabilities& value) -> bool
    {
        return mapper.MapObject("general", [&] { return mapper.Map("positionEncodings", value.positionEncodings); });
    }

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
        // positionEncoding?: PositionEncodingKind;
        std::string positionEncoding;

        // /**
        //  * Defines how text documents are synced. Is either a detailed structure
        //  * defining each notification or for backwards compatibility the
        //  * TextDocumentSyncKind number. If omitted it defaults to
        //  * `TextDocumentSyncKind.None`.
        //  */
        // textDocumentSync?: TextDocumentSyncOptions | TextDocumentSyncKind;
        TextDocumentSyncOptions textDocumentSync;

        // /**
        //  * The server provides completion support.
        //  */
        // completionProvider?: CompletionOptions;
        std::optional<CompletionOptions> completionProvider;

        // /**
        //  * The server provides hover support.
        //  */
        // hoverProvider ?: boolean | HoverOptions;
        bool hoverProvider;

        // /**
        //  * The server provides signature help support.
        //  */
        // signatureHelpProvider?: SignatureHelpOptions;
        std::optional<SignatureHelpOptions> signatureHelpProvider;

        // /**
        //  * The server provides go to declaration support.
        //  *
        //  * @since 3.14.0
        //  */
        // declarationProvider?: boolean | DeclarationOptions
        //     | DeclarationRegistrationOptions;
        bool declarationProvider;

        // /**
        //  * The server provides goto definition support.
        //  */
        // definitionProvider?: boolean | DefinitionOptions;
        bool definitionProvider;

        // /**
        //  * The server provides find references support.
        //  */
        // referencesProvider?: boolean | ReferenceOptions;
        bool referenceProvider;

        // /**
        //  * The server provides document symbol support.
        //  */
        // documentSymbolProvider ?: boolean | DocumentSymbolOptions;
        bool documentSymbolProvider;

        // /**
        //  * The server provides semantic tokens support.
        //  *
        //  * @since 3.16.0
        //  */
        // semanticTokensProvider?: SemanticTokensOptions
        //     | SemanticTokensRegistrationOptions;
        std::optional<SemanticTokenOptions> semanticTokensProvider;

        // /**
        //  * The server provides inlay hints.
        //  *
        //  * @since 3.17.0
        //  */
        // inlayHintProvider?: boolean | InlayHintOptions
        // 	    | InlayHintRegistrationOptions;
        std::optional<InlayHintOptions> inlayHintProvider;

        // /**
        //  * The server provides color provider support.
        //  *
        //  * @since 3.6.0
        //  */
        // colorProvider?: boolean | DocumentColorOptions
        //     | DocumentColorRegistrationOptions;
        bool colorProvider;

        // /**
        //  * The server provides rename support. RenameOptions may only be
        //  * specified if the client states that it supports
        //  * `prepareSupport` in its initial `initialize` request.
        //  */
        // renameProvider?: boolean | RenameOptions;
        std::optional<RenameOptions> renameProvider;
    };
    inline auto MapJson(ObjectToJsonMapper& mapper, const ServerCapabilities& value) -> bool
    {
        return mapper.Map("textDocumentSync", value.textDocumentSync) &&
               mapper.Map("completionProvider", value.completionProvider) &&
               mapper.Map("hoverProvider", value.hoverProvider) &&
               mapper.Map("signatureHelpProvider", value.signatureHelpProvider) &&
               mapper.Map("declarationProvider", value.declarationProvider) &&
               mapper.Map("definitionProvider", value.definitionProvider) &&
               mapper.Map("referenceProvider", value.referenceProvider) &&
               mapper.Map("documentSymbolProvider", value.documentSymbolProvider) &&
               mapper.Map("semanticTokensProvider", value.semanticTokensProvider) &&
               mapper.Map("inlayHintProvider", value.inlayHintProvider) &&
               mapper.Map("colorProvider", value.colorProvider) && mapper.Map("renameProvider", value.renameProvider);
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
    inline auto MapJson(ObjectFromJsonMapper& mapper, InitializeParams& value) -> bool
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
    inline auto MapJson(ObjectToJsonMapper& mapper, const InitializedResult& value) -> bool
    {
        return mapper.Map("capabilities", value.capabilities) && mapper.MapObject("serverInfo", [&] {
            return mapper.Map("name", value.serverInfo.name) && mapper.Map("version", value.serverInfo.version);
        });
    }

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
    inline auto MapJson(ObjectToJsonMapper& mapper, const InitializeError& value) -> bool
    {
        return mapper.Map("retry", value.retry);
    }

} // namespace glsld::lsp
