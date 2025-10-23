#include "ServerTestFixture.h"

#include "Feature/SignatureHelp.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockSignatureHelp(const ServerTestFixture& fixture, TextPosition pos,
                              const SignatureHelpConfig& config = {}) -> std::optional<lsp::SignatureHelp>
{
    return HandleSignatureHelp(config, fixture.GetLanguageQueryInfo(),
                               lsp::SignatureHelpParams{
                                   .textDocument = {"MockDocument"},
                                   .position     = ToLspPosition(pos),
                               });
}

TEST_CASE_METHOD(ServerTestFixture, "SignatureHelpTest")
{
    // TODO: implement tests
}