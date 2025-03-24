#pragma once
#include "Server/LanguageQueryInfo.h"
#include "Server/Protocol.h"

namespace glsld::lsp
{
    inline constexpr const char* LSPMethod_Rename        = "textDocument/rename";
    inline constexpr const char* LSPMethod_PrepareRename = "textDocument/prepareRename";

    struct RenameParams : TextDocumentPositionParams
    {
        // /**
        //  * The new name of the symbol. If the given name is not valid the
        //  * request must return a [ResponseError](#ResponseError) with an
        //  * appropriate message set.
        //  */
        //  newName: string;
        std::string newName;
    };

    struct PrepareRenameParams : TextDocumentPositionParams
    {
    };

} // namespace glsld::lsp