#include "ServerTestFixture.h"

#include "Feature/Reference.h"
#include "Support/SourceText.h"

using namespace glsld;

static auto MockReferences(const ServerTestFixture& fixture, TextPosition pos, const ReferenceConfig& config = {})
    -> std::vector<lsp::Location>
{
    return HandleReferences(config, fixture.GetLanguageQueryInfo(),
                            lsp::ReferenceParams{
                                .textDocument = {"MockDocument"},
                                .position     = ToLspPosition(pos),
                            });
}

TEST_CASE_METHOD(ServerTestFixture, "ReferenceTest")
{
}