#include "Config.h"

#include <catch2/catch_test_macros.hpp>

using namespace glsld;

TEST_CASE("ConfigTest")
{
    auto config       = R"(
{
    "languageService": {
        "declaration": {},
        "reference": {},
        "hover": {},
        "foldingRange": {},
        "documentSymbol": {},
        "semanticToken": {},
        "inlayHint": {
            "enableArgumentNameHint": true,
            "enableImplicitCastHint": true,
            "enableBlockEndHint": true,
            "blockEndHintLineThreshold": 4
        },
        "completion": {},
        "diagnostic": {},
        "signatureHelp": {}
    },
    "loggingLevel": "debug"
}
)";
    auto parsedConfig = ParseLanguageServerConfig(config);
    REQUIRE(parsedConfig.has_value());

    // Inlay hint
    REQUIRE(parsedConfig->languageService.inlayHint.enableArgumentNameHint);
    REQUIRE(parsedConfig->languageService.inlayHint.enableImplicitCastHint);
    REQUIRE(parsedConfig->languageService.inlayHint.enableBlockEndHint);
    REQUIRE(parsedConfig->languageService.inlayHint.blockEndHintLineThreshold == 4);

    // Logging
    REQUIRE(parsedConfig->loggingLevel == LoggingLevel::Debug);
}