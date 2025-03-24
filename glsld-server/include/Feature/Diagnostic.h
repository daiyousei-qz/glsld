#pragma once
#include "Server/Protocol.h"
#include "Server/LanguageQueryInfo.h"

namespace glsld::lsp
{
    inline constexpr const char* LSPMethod_PullDiagnostic = "textDocument/diagnostic";

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

    // /**
    //  * Diagnostic options.
    //  *
    //  * @since 3.17.0
    //  */
    struct DiagnosticOptions
    {
        // /**
        //  * An optional identifier under which the diagnostics are
        //  * managed by the client.
        //  */
        //  identifier?: string;

        //  /**
        //   * Whether the language has inter file dependencies meaning that
        //   * editing code in one file can result in a different diagnostic
        //   * set in another file. Inter file dependencies are common for
        //   * most programming languages and typically uncommon for linters.
        //   */
        //  interFileDependencies: boolean;

        //  /**
        //   * The server provides support for workspace diagnostics as well.
        //   */
        //  workspaceDiagnostics: boolean;
    };

    // /**
    //  * Parameters of the document diagnostic request.
    //  *
    //  * @since 3.17.0
    //  */
    struct DocumentDiagnosticParams
    {
        // /**
        //  * The text document.
        //  */
        //  textDocument: TextDocumentIdentifier;
        TextDocumentIdentifier textDocument;

        //  /**
        //   * The additional identifier  provided during registration.
        //   */
        //  identifier?: string;

        //  /**
        //   * The result id of a previous response if provided.
        //   */
        //  previousResultId?: string;
    };
} // namespace glsld::lsp