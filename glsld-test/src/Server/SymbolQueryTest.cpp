#include "Support/StringView.h"
#include "ServerTestFixture.h"

#include "Server/LanguageQueryInfo.h"

#include <optional>
#include <variant>

using namespace glsld;

struct MacroSymbolExpectedResult
{
    PPMacroOccurrenceType occurrenceType;
};

struct HeaderSymbolExpectedResult
{
    std::string headerAbsolutePath;
};

struct AstSymbolExpectedResult
{
    AstNodeTag tag;
};

struct SymbolQueryExpectedResult
{
    SymbolDeclType symbolType;

    std::string spelledText;

    std::pair<StringView, StringView> spelledRangeLabels;

    std::variant<MacroSymbolExpectedResult, HeaderSymbolExpectedResult, AstSymbolExpectedResult, std::monostate>
        symbolOccurrence = std::monostate{};

    bool isDeclaration = false;
};

TEST_CASE_METHOD(ServerTestFixture, "SymbolQueryTest")
{
    auto checkSymbol = [this](StringView labelPos, SymbolQueryExpectedResult expectedResult) {
        auto queryResult = GetLanguageQueryInfo().QuerySymbolByPosition(GetLabelledPosition(labelPos));
        REQUIRE(queryResult.has_value());
        REQUIRE(queryResult->symbolType == expectedResult.symbolType);
        REQUIRE(queryResult->spelledText == expectedResult.spelledText);
        REQUIRE(queryResult->spelledRange ==
                GetLabelledRange(expectedResult.spelledRangeLabels.first, expectedResult.spelledRangeLabels.second));
        if (auto expectedMacro = std::get_if<MacroSymbolExpectedResult>(&expectedResult.symbolOccurrence)) {
            REQUIRE(queryResult->ppSymbolOccurrence != nullptr);
            REQUIRE(queryResult->ppSymbolOccurrence->GetMacroInfo() != nullptr);

            const auto& macroInfo = *queryResult->ppSymbolOccurrence->GetMacroInfo();
            REQUIRE(macroInfo.occurrenceType == expectedMacro->occurrenceType);
        }
        else if (auto expectedHeader = std::get_if<HeaderSymbolExpectedResult>(&expectedResult.symbolOccurrence)) {
            REQUIRE(queryResult->ppSymbolOccurrence != nullptr);
            REQUIRE(queryResult->ppSymbolOccurrence->GetHeaderNameInfo() != nullptr);

            const auto& headerNameInfo = *queryResult->ppSymbolOccurrence->GetHeaderNameInfo();
            REQUIRE(headerNameInfo.headerAbsolutePath == expectedHeader->headerAbsolutePath);
        }
        else if (auto expectedAst = std::get_if<AstSymbolExpectedResult>(&expectedResult.symbolOccurrence)) {
            REQUIRE(queryResult->astSymbolOccurrence != nullptr);
            REQUIRE(queryResult->astSymbolOccurrence->GetTag() == expectedAst->tag);
        }
        REQUIRE(queryResult->isDeclaration == expectedResult.isDeclaration);
    };

    SECTION("HeaderName")
    {
        CompileLabelledSource(R"(
            #include ^[header1.begin]"test.h"^[header1.end]
            #include ^[header2.begin]<test.h>^[header2.end]
        )");

        checkSymbol("header1.begin", SymbolQueryExpectedResult{
                                         .symbolType         = SymbolDeclType::HeaderName,
                                         .spelledText        = "\"test.h\"",
                                         .spelledRangeLabels = {"header1.begin", "header1.end"},
                                         .symbolOccurrence =
                                             HeaderSymbolExpectedResult{
                                                 .headerAbsolutePath = "",
                                             },
                                     });
        checkSymbol("header2.begin", SymbolQueryExpectedResult{
                                         .symbolType         = SymbolDeclType::HeaderName,
                                         .spelledText        = "<test.h>",
                                         .spelledRangeLabels = {"header2.begin", "header2.end"},
                                         .symbolOccurrence =
                                             HeaderSymbolExpectedResult{
                                                 .headerAbsolutePath = "",
                                             },
                                     });
    }

    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #define ^[macro1.decl.begin]MACRO1^[macro1.decl.end]
            #define ^[macro2.decl.begin]MACRO2^[macro2.decl.end] 1
            #define ^[macro3.decl.begin]MACRO3^[macro3.decl.end](X) X

            #ifdef ^[macro1.use.begin]MACRO1^[macro1.use.end]
            #endif
            #if defined ^[macro2.use.begin]MACRO2^[macro2.use.end] && ^[macro3.use.begin]MACRO3^[macro3.use.end](0)
            #endif

            #undef ^[macro1.undef.begin]MACRO1^[macro1.undef.end]
            #undef ^[macro4.undef.begin]MACRO4^[macro4.undef.end]
        )");

        checkSymbol("macro1.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::Macro,
                                             .spelledText        = "MACRO1",
                                             .spelledRangeLabels = {"macro1.decl.begin", "macro1.decl.end"},
                                             .symbolOccurrence =
                                                 MacroSymbolExpectedResult{
                                                     .occurrenceType = PPMacroOccurrenceType::Define,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("macro2.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::Macro,
                                             .spelledText        = "MACRO2",
                                             .spelledRangeLabels = {"macro2.decl.begin", "macro2.decl.end"},
                                             .symbolOccurrence =
                                                 MacroSymbolExpectedResult{
                                                     .occurrenceType = PPMacroOccurrenceType::Define,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("macro3.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::Macro,
                                             .spelledText        = "MACRO3",
                                             .spelledRangeLabels = {"macro3.decl.begin", "macro3.decl.end"},
                                             .symbolOccurrence =
                                                 MacroSymbolExpectedResult{
                                                     .occurrenceType = PPMacroOccurrenceType::Define,
                                                 },
                                             .isDeclaration = true,
                                         });

        checkSymbol("macro1.use.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::Macro,
                                            .spelledText        = "MACRO1",
                                            .spelledRangeLabels = {"macro1.use.begin", "macro1.use.end"},
                                            .symbolOccurrence =
                                                MacroSymbolExpectedResult{
                                                    .occurrenceType = PPMacroOccurrenceType::IfDef,
                                                },
                                        });
        checkSymbol("macro2.use.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::Macro,
                                            .spelledText        = "MACRO2",
                                            .spelledRangeLabels = {"macro2.use.begin", "macro2.use.end"},
                                            .symbolOccurrence =
                                                MacroSymbolExpectedResult{
                                                    .occurrenceType = PPMacroOccurrenceType::IfDef,
                                                },
                                        });
        checkSymbol("macro3.use.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::Macro,
                                            .spelledText        = "MACRO3",
                                            .spelledRangeLabels = {"macro3.use.begin", "macro3.use.end"},
                                            .symbolOccurrence =
                                                MacroSymbolExpectedResult{
                                                    .occurrenceType = PPMacroOccurrenceType::Expand,
                                                },
                                        });

        checkSymbol("macro1.undef.begin", SymbolQueryExpectedResult{
                                              .symbolType         = SymbolDeclType::Macro,
                                              .spelledText        = "MACRO1",
                                              .spelledRangeLabels = {"macro1.undef.begin", "macro1.undef.end"},
                                              .symbolOccurrence =
                                                  MacroSymbolExpectedResult{
                                                      .occurrenceType = PPMacroOccurrenceType::Undef,
                                                  },
                                          });
        checkSymbol("macro4.undef.begin", SymbolQueryExpectedResult{
                                              .symbolType         = SymbolDeclType::Macro,
                                              .spelledText        = "MACRO4",
                                              .spelledRangeLabels = {"macro4.undef.begin", "macro4.undef.end"},
                                              .symbolOccurrence =
                                                  MacroSymbolExpectedResult{
                                                      .occurrenceType = PPMacroOccurrenceType::Undef,
                                                  },
                                          });
    }

    SECTION("LayoutQualifier")
    {
        CompileLabelledSource(R"(
            layout(^[qual.binding.begin]binding^[qual.binding.end] = 0, ^[qual.rgba.begin]rgba^[qual.rgba.end]) uniform image2D img;
        )");

        checkSymbol("qual.binding.begin", SymbolQueryExpectedResult{
                                              .symbolType         = SymbolDeclType::LayoutQualifier,
                                              .spelledText        = "binding",
                                              .spelledRangeLabels = {"qual.binding.begin", "qual.binding.end"},
                                          });
        checkSymbol("qual.rgba.begin", SymbolQueryExpectedResult{
                                           .symbolType         = SymbolDeclType::LayoutQualifier,
                                           .spelledText        = "rgba",
                                           .spelledRangeLabels = {"qual.rgba.begin", "qual.rgba.end"},
                                       });
    }

    SECTION("Struct")
    {
        CompileLabelledSource(R"(
            struct ^[struct.decl.begin]S^[struct.decl.end]
            {
                int ^[member.decl.begin]member^[member.decl.end];
            };

            void foo()
            {
                ^[struct.use.begin]S^[struct.use.end] s;
                s.^[member.use.begin]member^[member.use.end];
                s.^[unknown.member.use.begin]unknown^[unknown.member.use.end];

                ^[unknown.type.use.begin]Unknown^[unknown.type.use.end] u;
            }
        )");

        checkSymbol("struct.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::Type,
                                             .spelledText        = "S",
                                             .spelledRangeLabels = {"struct.decl.begin", "struct.decl.end"},
                                             .symbolOccurrence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstStructDecl,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("member.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::StructMember,
                                             .spelledText        = "member",
                                             .spelledRangeLabels = {"member.decl.begin", "member.decl.end"},
                                             .symbolOccurrence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstStructFieldDeclaratorDecl,
                                                 },
                                             .isDeclaration = true,
                                         });

        checkSymbol("struct.use.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::Type,
                                            .spelledText        = "S",
                                            .spelledRangeLabels = {"struct.use.begin", "struct.use.end"},
                                            .symbolOccurrence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstQualType,
                                                },
                                        });
        checkSymbol("member.use.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::StructMember,
                                            .spelledText        = "member",
                                            .spelledRangeLabels = {"member.use.begin", "member.use.end"},
                                            .symbolOccurrence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstFieldAccessExpr,
                                                },
                                        });

        checkSymbol("unknown.member.use.begin",
                    SymbolQueryExpectedResult{
                        .symbolType         = SymbolDeclType::StructMember,
                        .spelledText        = "unknown",
                        .spelledRangeLabels = {"unknown.member.use.begin", "unknown.member.use.end"},
                        .symbolOccurrence =
                            AstSymbolExpectedResult{
                                .tag = AstNodeTag::AstFieldAccessExpr,
                            },
                    });
        checkSymbol("unknown.type.use.begin",
                    SymbolQueryExpectedResult{
                        .symbolType         = SymbolDeclType::Type,
                        .spelledText        = "Unknown",
                        .spelledRangeLabels = {"unknown.type.use.begin", "unknown.type.use.end"},
                        .symbolOccurrence =
                            AstSymbolExpectedResult{
                                .tag = AstNodeTag::AstQualType,
                            },
                    });
    }

    SECTION("Block")
    {
        CompileLabelledSource(R"(
            uniform ^[ubo.decl.begin]UBO^[ubo.decl.end]
            {
                int ^[ubo.member.decl.begin]value^[ubo.member.decl.end];
            } ^[ubo.instance.decl.begin]block^[ubo.instance.decl.end];

            buffer ^[ssbo.decl.begin]SSBO^[ssbo.decl.end]
            {
                int ^[ssbo.member.decl.begin]test^[ssbo.member.decl.end][];
            };

            int foo()
            {
                int index = ^[ubo.instance.use.begin]block^[ubo.instance.use.end].^[ubo.member.use.begin]value^[ubo.member.use.end];
                return ^[ssbo.member.use.begin]test^[ssbo.member.use.end][index];
            }
        )");

        checkSymbol("ubo.decl.begin", SymbolQueryExpectedResult{
                                          .symbolType         = SymbolDeclType::Block,
                                          .spelledText        = "UBO",
                                          .spelledRangeLabels = {"ubo.decl.begin", "ubo.decl.end"},
                                          .symbolOccurrence =
                                              AstSymbolExpectedResult{
                                                  .tag = AstNodeTag::AstInterfaceBlockDecl,
                                              },
                                          .isDeclaration = true,
                                      });
        checkSymbol("ubo.member.decl.begin", SymbolQueryExpectedResult{
                                                 .symbolType         = SymbolDeclType::BlockMember,
                                                 .spelledText        = "value",
                                                 .spelledRangeLabels = {"ubo.member.decl.begin", "ubo.member.decl.end"},
                                                 .symbolOccurrence =
                                                     AstSymbolExpectedResult{
                                                         .tag = AstNodeTag::AstBlockFieldDeclaratorDecl,
                                                     },
                                                 .isDeclaration = true,
                                             });
        checkSymbol("ubo.instance.decl.begin",
                    SymbolQueryExpectedResult{
                        .symbolType         = SymbolDeclType::BlockInstance,
                        .spelledText        = "block",
                        .spelledRangeLabels = {"ubo.instance.decl.begin", "ubo.instance.decl.end"},
                        .symbolOccurrence =
                            AstSymbolExpectedResult{
                                .tag = AstNodeTag::AstInterfaceBlockDecl,
                            },
                        .isDeclaration = true,
                    });
        checkSymbol("ssbo.decl.begin", SymbolQueryExpectedResult{
                                           .symbolType         = SymbolDeclType::Block,
                                           .spelledText        = "SSBO",
                                           .spelledRangeLabels = {"ssbo.decl.begin", "ssbo.decl.end"},
                                           .symbolOccurrence =
                                               AstSymbolExpectedResult{
                                                   .tag = AstNodeTag::AstInterfaceBlockDecl,
                                               },
                                           .isDeclaration = true,
                                       });
        checkSymbol("ssbo.member.decl.begin",
                    SymbolQueryExpectedResult{
                        .symbolType         = SymbolDeclType::BlockMember,
                        .spelledText        = "test",
                        .spelledRangeLabels = {"ssbo.member.decl.begin", "ssbo.member.decl.end"},
                        .symbolOccurrence =
                            AstSymbolExpectedResult{
                                .tag = AstNodeTag::AstBlockFieldDeclaratorDecl,
                            },
                        .isDeclaration = true,
                    });

        checkSymbol("ubo.instance.use.begin",
                    SymbolQueryExpectedResult{
                        .symbolType         = SymbolDeclType::BlockInstance,
                        .spelledText        = "block",
                        .spelledRangeLabels = {"ubo.instance.use.begin", "ubo.instance.use.end"},
                        .symbolOccurrence =
                            AstSymbolExpectedResult{
                                .tag = AstNodeTag::AstNameAccessExpr,
                            },
                    });
        checkSymbol("ubo.member.use.begin", SymbolQueryExpectedResult{
                                                .symbolType         = SymbolDeclType::BlockMember,
                                                .spelledText        = "value",
                                                .spelledRangeLabels = {"ubo.member.use.begin", "ubo.member.use.end"},
                                                .symbolOccurrence =
                                                    AstSymbolExpectedResult{
                                                        .tag = AstNodeTag::AstFieldAccessExpr,
                                                    },
                                            });
        checkSymbol("ssbo.member.use.begin", SymbolQueryExpectedResult{
                                                 .symbolType         = SymbolDeclType::BlockMember,
                                                 .spelledText        = "test",
                                                 .spelledRangeLabels = {"ssbo.member.use.begin", "ssbo.member.use.end"},
                                                 .symbolOccurrence =
                                                     AstSymbolExpectedResult{
                                                         .tag = AstNodeTag::AstNameAccessExpr,
                                                     },
                                             });
    }

    SECTION("Variable")
    {
        CompileLabelledSource(R"(
            int ^[global.decl.begin]global^[global.decl.end] = 41;

            int foo()
            {
                int ^[local.decl.begin]local^[local.decl.end] = 42;
                return ^[global.use.begin]global^[global.use.end] +
                       ^[local.use.begin]local^[local.use.end] +
                       ^[unknown.use.begin]unknown^[unknown.use.end];
            }
        )");

        checkSymbol("global.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::GlobalVariable,
                                             .spelledText        = "global",
                                             .spelledRangeLabels = {"global.decl.begin", "global.decl.end"},
                                             .symbolOccurrence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstVariableDeclaratorDecl,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("local.decl.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::LocalVariable,
                                            .spelledText        = "local",
                                            .spelledRangeLabels = {"local.decl.begin", "local.decl.end"},
                                            .symbolOccurrence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstVariableDeclaratorDecl,
                                                },
                                            .isDeclaration = true,
                                        });
        checkSymbol("global.use.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::GlobalVariable,
                                            .spelledText        = "global",
                                            .spelledRangeLabels = {"global.use.begin", "global.use.end"},
                                            .symbolOccurrence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstNameAccessExpr,
                                                },
                                        });
        checkSymbol("local.use.begin", SymbolQueryExpectedResult{
                                           .symbolType         = SymbolDeclType::LocalVariable,
                                           .spelledText        = "local",
                                           .spelledRangeLabels = {"local.use.begin", "local.use.end"},
                                           .symbolOccurrence =
                                               AstSymbolExpectedResult{
                                                   .tag = AstNodeTag::AstNameAccessExpr,
                                               },
                                       });
        checkSymbol("unknown.use.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::GlobalVariable,
                                             .spelledText        = "unknown",
                                             .spelledRangeLabels = {"unknown.use.begin", "unknown.use.end"},
                                             .symbolOccurrence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstNameAccessExpr,
                                                 },
                                         });
    }

    SECTION("Parameter")
    {
        CompileLabelledSource(R"(
            int identity(int ^[param.decl.begin]param^[param.decl.end])
            {
                return ^[param.use.begin]param^[param.use.end];
            }
        )");

        checkSymbol("param.decl.begin", SymbolQueryExpectedResult{
                                            .symbolType         = SymbolDeclType::Parameter,
                                            .spelledText        = "param",
                                            .spelledRangeLabels = {"param.decl.begin", "param.decl.end"},
                                            .symbolOccurrence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstParamDecl,
                                                },
                                            .isDeclaration = true,
                                        });
        checkSymbol("param.use.begin", SymbolQueryExpectedResult{
                                           .symbolType         = SymbolDeclType::Parameter,
                                           .spelledText        = "param",
                                           .spelledRangeLabels = {"param.use.begin", "param.use.end"},
                                           .symbolOccurrence =
                                               AstSymbolExpectedResult{
                                                   .tag = AstNodeTag::AstNameAccessExpr,
                                               },
                                       });
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            void ^[foo.decl.begin]foo^[foo.decl.end]();

            void bar()
            {
                ^[foo.use.begin]foo^[foo.use.end]();
                ^[unknown.use.begin]unknown^[unknown.use.end]();
            }

            void ^[foo.decl.def.begin]foo^[foo.decl.def.end]()
            {
            }
        )");

        checkSymbol("foo.decl.begin", SymbolQueryExpectedResult{
                                          .symbolType         = SymbolDeclType::Function,
                                          .spelledText        = "foo",
                                          .spelledRangeLabels = {"foo.decl.begin", "foo.decl.end"},
                                          .symbolOccurrence =
                                              AstSymbolExpectedResult{
                                                  .tag = AstNodeTag::AstFunctionDecl,
                                              },
                                          .isDeclaration = true,
                                      });
        checkSymbol("foo.use.begin", SymbolQueryExpectedResult{
                                         .symbolType         = SymbolDeclType::Function,
                                         .spelledText        = "foo",
                                         .spelledRangeLabels = {"foo.use.begin", "foo.use.end"},
                                         .symbolOccurrence =
                                             AstSymbolExpectedResult{
                                                 .tag = AstNodeTag::AstFunctionCallExpr,
                                             },
                                     });
        checkSymbol("unknown.use.begin", SymbolQueryExpectedResult{
                                             .symbolType         = SymbolDeclType::Function,
                                             .spelledText        = "unknown",
                                             .spelledRangeLabels = {"unknown.use.begin", "unknown.use.end"},
                                             .symbolOccurrence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstFunctionCallExpr,
                                                 },
                                         });
        checkSymbol("foo.decl.def.begin", SymbolQueryExpectedResult{
                                              .symbolType         = SymbolDeclType::Function,
                                              .spelledText        = "foo",
                                              .spelledRangeLabels = {"foo.decl.def.begin", "foo.decl.def.end"},
                                              .symbolOccurrence =
                                                  AstSymbolExpectedResult{
                                                      .tag = AstNodeTag::AstFunctionDecl,
                                                  },
                                              .isDeclaration = true,
                                          });
    }

    SECTION("SwizzleName")
    {
        CompileLabelledSource(R"(
            void foo()
            {
                vec4 v;
                v.^[swizzle1.use.begin]x^[swizzle1.use.end];
                v.^[swizzle2.use.begin]xyzw^[swizzle2.use.end];
            }
        )");

        checkSymbol("swizzle1.use.begin", SymbolQueryExpectedResult{
                                              .symbolType         = SymbolDeclType::Swizzle,
                                              .spelledText        = "x",
                                              .spelledRangeLabels = {"swizzle1.use.begin", "swizzle1.use.end"},
                                              .symbolOccurrence =
                                                  AstSymbolExpectedResult{
                                                      .tag = AstNodeTag::AstSwizzleAccessExpr,
                                                  },
                                          });
        checkSymbol("swizzle2.use.begin", SymbolQueryExpectedResult{
                                              .symbolType         = SymbolDeclType::Swizzle,
                                              .spelledText        = "xyzw",
                                              .spelledRangeLabels = {"swizzle2.use.begin", "swizzle2.use.end"},
                                              .symbolOccurrence =
                                                  AstSymbolExpectedResult{
                                                      .tag = AstNodeTag::AstSwizzleAccessExpr,
                                                  },
                                          });
    }
}