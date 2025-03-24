#include "Server/Config.h"

#include <catch2/catch_test_macros.hpp>

using namespace glsld;

TEST_CASE("ConfigTest")
{
    auto config       = R"(
{
    "languageService": {
        "declaration": {
            "enable": true
        },
        "reference": {
            "enable": true
        },
        "hover": {
            "enable": true
        },
        "foldingRange": {
            "enable": true
        },
        "documentSymbol": {
            "enable": true
        },
        "semanticToken": {
            "enable": true
        },
        "inlayHint": {
            "enable": true,
            "enableArgumentNameHint": true,
            "enableImplicitCastHint": true,
            "enableBlockEndHint": true,
            "blockEndHintLineThreshold": 4
        },
        "completion": {
            "enable": true
        },
        "diagnostic": {
            "enable": true
        },
        "signatureHelp": {
            "enable": true
        }
    },
    "loggingLevel": "Debug"
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