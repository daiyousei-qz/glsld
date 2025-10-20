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

    // TextRange spelledRange;

    std::variant<MacroSymbolExpectedResult, HeaderSymbolExpectedResult, AstSymbolExpectedResult, std::monostate>
        symbolOccurence = std::monostate{};

    bool isDeclaration = false;
};

TEST_CASE_METHOD(ServerTestFixture, "SymbolQueryTest")
{
    auto checkSymbol = [this](StringView labelPos, SymbolQueryExpectedResult expectedResult) {
        auto queryResult = GetLanguageQueryInfo().QuerySymbolByPosition(GetLabelledPosition(labelPos));
        REQUIRE(queryResult.has_value());
        REQUIRE(queryResult->symbolType == expectedResult.symbolType);
        REQUIRE(queryResult->spelledText == expectedResult.spelledText);
        if (auto expectedMacro = std::get_if<MacroSymbolExpectedResult>(&expectedResult.symbolOccurence)) {
            REQUIRE(queryResult->ppSymbolOccurrence != nullptr);
            REQUIRE(queryResult->ppSymbolOccurrence->GetMacroInfo() != nullptr);

            const auto& macroInfo = *queryResult->ppSymbolOccurrence->GetMacroInfo();
            REQUIRE(macroInfo.occurrenceType == expectedMacro->occurrenceType);
        }
        else if (auto expectedHeader = std::get_if<HeaderSymbolExpectedResult>(&expectedResult.symbolOccurence)) {
            REQUIRE(queryResult->ppSymbolOccurrence != nullptr);
            REQUIRE(queryResult->ppSymbolOccurrence->GetHeaderNameInfo() != nullptr);

            const auto& headerNameInfo = *queryResult->ppSymbolOccurrence->GetHeaderNameInfo();
            REQUIRE(headerNameInfo.headerAbsolutePath == expectedHeader->headerAbsolutePath);
        }
        else if (auto expectedAst = std::get_if<AstSymbolExpectedResult>(&expectedResult.symbolOccurence)) {
            REQUIRE(queryResult->astSymbolOccurrence != nullptr);
            REQUIRE(queryResult->astSymbolOccurrence->GetTag() == expectedAst->tag);
        }
        REQUIRE(queryResult->isDeclaration == expectedResult.isDeclaration);
    };

    SECTION("HeaderName")
    {
        CompileLabelledSource(R"(
            #include ^[header1.begin]"test.h"
            #include ^[header2.begin]<test.h>
        )");

        checkSymbol("header1.begin", SymbolQueryExpectedResult{
                                         .symbolType  = SymbolDeclType::HeaderName,
                                         .spelledText = "\"test.h\"",
                                         .symbolOccurence =
                                             HeaderSymbolExpectedResult{
                                                 .headerAbsolutePath = "",
                                             },
                                     });
        checkSymbol("header2.begin", SymbolQueryExpectedResult{
                                         .symbolType  = SymbolDeclType::HeaderName,
                                         .spelledText = "<test.h>",
                                         .symbolOccurence =
                                             HeaderSymbolExpectedResult{
                                                 .headerAbsolutePath = "",
                                             },
                                     });
    }

    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #define ^[macro1.decl.begin]MACRO1
            #define ^[macro2.decl.begin]MACRO2 1
            #define ^[macro3.decl.begin]MACRO3(X) X

            #ifdef ^[macro1.use.begin]MACRO1
            #endif
            #if defined ^[macro2.use.begin]MACRO2 && ^[macro3.use.begin]MACRO3(0)
            #endif

            #undef ^[macro1.undef.begin]MACRO1
            #undef ^[macro4.undef.begin]MACRO4
        )");

        checkSymbol("macro1.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::Macro,
                                             .spelledText = "MACRO1",
                                             .symbolOccurence =
                                                 MacroSymbolExpectedResult{
                                                     .occurrenceType = PPMacroOccurrenceType::Define,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("macro2.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::Macro,
                                             .spelledText = "MACRO2",
                                             .symbolOccurence =
                                                 MacroSymbolExpectedResult{
                                                     .occurrenceType = PPMacroOccurrenceType::Define,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("macro3.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::Macro,
                                             .spelledText = "MACRO3",
                                             .symbolOccurence =
                                                 MacroSymbolExpectedResult{
                                                     .occurrenceType = PPMacroOccurrenceType::Define,
                                                 },
                                             .isDeclaration = true,
                                         });

        checkSymbol("macro1.use.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::Macro,
                                            .spelledText = "MACRO1",
                                            .symbolOccurence =
                                                MacroSymbolExpectedResult{
                                                    .occurrenceType = PPMacroOccurrenceType::IfDef,
                                                },
                                        });
        checkSymbol("macro2.use.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::Macro,
                                            .spelledText = "MACRO2",
                                            .symbolOccurence =
                                                MacroSymbolExpectedResult{
                                                    .occurrenceType = PPMacroOccurrenceType::IfDef,
                                                },
                                        });
        checkSymbol("macro3.use.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::Macro,
                                            .spelledText = "MACRO3",
                                            .symbolOccurence =
                                                MacroSymbolExpectedResult{
                                                    .occurrenceType = PPMacroOccurrenceType::Expand,
                                                },
                                        });

        checkSymbol("macro1.undef.begin", SymbolQueryExpectedResult{
                                              .symbolType  = SymbolDeclType::Macro,
                                              .spelledText = "MACRO1",
                                              .symbolOccurence =
                                                  MacroSymbolExpectedResult{
                                                      .occurrenceType = PPMacroOccurrenceType::Undef,
                                                  },
                                          });
        checkSymbol("macro4.undef.begin", SymbolQueryExpectedResult{
                                              .symbolType  = SymbolDeclType::Macro,
                                              .spelledText = "MACRO4",
                                              .symbolOccurence =
                                                  MacroSymbolExpectedResult{
                                                      .occurrenceType = PPMacroOccurrenceType::Undef,
                                                  },
                                          });
    }

    SECTION("LayoutQualifier")
    {
        CompileLabelledSource(R"(
            layout(^[qual.binding.begin]binding = 0, ^[qual.rgba.begin]rgba) uniform image2D img;
        )");

        checkSymbol("qual.binding.begin", SymbolQueryExpectedResult{
                                              .symbolType  = SymbolDeclType::LayoutQualifier,
                                              .spelledText = "binding",
                                          });
        checkSymbol("qual.rgba.begin", SymbolQueryExpectedResult{
                                           .symbolType  = SymbolDeclType::LayoutQualifier,
                                           .spelledText = "rgba",
                                       });
    }

    SECTION("Struct")
    {
        CompileLabelledSource(R"(
            struct ^[struct.decl.begin]S
            {
                int ^[member.decl.begin]member;
            };

            void foo()
            {
                ^[struct.use.begin]S s;
                s.^[member.use.begin]member;
                s.^[unknown.member.use.begin]unknown;

                ^[unknown.type.use.begin]Unknown u;
            }
        )");

        checkSymbol("struct.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::Type,
                                             .spelledText = "S",
                                             .symbolOccurence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstStructDecl,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("member.decl.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::StructMember,
                                             .spelledText = "member",
                                             .symbolOccurence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstStructFieldDeclaratorDecl,
                                                 },
                                             .isDeclaration = true,
                                         });

        checkSymbol("struct.use.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::Type,
                                            .spelledText = "S",
                                            .symbolOccurence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstQualType,
                                                },
                                        });
        checkSymbol("member.use.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::StructMember,
                                            .spelledText = "member",
                                            .symbolOccurence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstFieldAccessExpr,
                                                },
                                        });

        checkSymbol("unknown.member.use.begin", SymbolQueryExpectedResult{
                                                    .symbolType  = SymbolDeclType::StructMember,
                                                    .spelledText = "unknown",
                                                    .symbolOccurence =
                                                        AstSymbolExpectedResult{
                                                            .tag = AstNodeTag::AstFieldAccessExpr,
                                                        },
                                                });
        checkSymbol("unknown.type.use.begin", SymbolQueryExpectedResult{
                                                  .symbolType  = SymbolDeclType::Type,
                                                  .spelledText = "Unknown",
                                                  .symbolOccurence =
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
                int ^[ssbo.member.decl.begin]test[]^[ssbo.member.decl.end];
            };

            int foo()
            {
                int index = ^[ubo.instance.use.begin]block^[ubo.instance.use.end].^[ubo.member.use.begin]value^[ubo.member.use.end];
                return ^[ssbo.member.use.begin]test^[ssbo.member.use.end][index];
            }
        )");

        checkSymbol("ubo.decl.begin", SymbolQueryExpectedResult{
                                          .symbolType  = SymbolDeclType::Block,
                                          .spelledText = "UBO",
                                          .symbolOccurence =
                                              AstSymbolExpectedResult{
                                                  .tag = AstNodeTag::AstInterfaceBlockDecl,
                                              },
                                          .isDeclaration = true,
                                      });
        checkSymbol("ubo.member.decl.begin", SymbolQueryExpectedResult{
                                                 .symbolType  = SymbolDeclType::BlockMember,
                                                 .spelledText = "value",
                                                 .symbolOccurence =
                                                     AstSymbolExpectedResult{
                                                         .tag = AstNodeTag::AstBlockFieldDeclaratorDecl,
                                                     },
                                                 .isDeclaration = true,
                                             });
        checkSymbol("ubo.instance.decl.begin", SymbolQueryExpectedResult{
                                                   .symbolType  = SymbolDeclType::BlockInstance,
                                                   .spelledText = "block",
                                                   .symbolOccurence =
                                                       AstSymbolExpectedResult{
                                                           .tag = AstNodeTag::AstInterfaceBlockDecl,
                                                       },
                                                   .isDeclaration = true,
                                               });
        checkSymbol("ssbo.decl.begin", SymbolQueryExpectedResult{
                                           .symbolType  = SymbolDeclType::Block,
                                           .spelledText = "SSBO",
                                           .symbolOccurence =
                                               AstSymbolExpectedResult{
                                                   .tag = AstNodeTag::AstInterfaceBlockDecl,
                                               },
                                           .isDeclaration = true,
                                       });
        checkSymbol("ssbo.member.decl.begin", SymbolQueryExpectedResult{
                                                  .symbolType  = SymbolDeclType::BlockMember,
                                                  .spelledText = "test",
                                                  .symbolOccurence =
                                                      AstSymbolExpectedResult{
                                                          .tag = AstNodeTag::AstBlockFieldDeclaratorDecl,
                                                      },
                                                  .isDeclaration = true,
                                              });

        checkSymbol("ubo.instance.use.begin", SymbolQueryExpectedResult{
                                                  .symbolType  = SymbolDeclType::BlockInstance,
                                                  .spelledText = "block",
                                                  .symbolOccurence =
                                                      AstSymbolExpectedResult{
                                                          .tag = AstNodeTag::AstNameAccessExpr,
                                                      },
                                              });
        checkSymbol("ubo.member.use.begin", SymbolQueryExpectedResult{
                                                .symbolType  = SymbolDeclType::BlockMember,
                                                .spelledText = "value",
                                                .symbolOccurence =
                                                    AstSymbolExpectedResult{
                                                        .tag = AstNodeTag::AstFieldAccessExpr,
                                                    },
                                            });
        checkSymbol("ssbo.member.use.begin", SymbolQueryExpectedResult{
                                                 .symbolType  = SymbolDeclType::BlockMember,
                                                 .spelledText = "test",
                                                 .symbolOccurence =
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
                                             .symbolType  = SymbolDeclType::GlobalVariable,
                                             .spelledText = "global",
                                             .symbolOccurence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstVariableDeclaratorDecl,
                                                 },
                                             .isDeclaration = true,
                                         });
        checkSymbol("local.decl.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::LocalVariable,
                                            .spelledText = "local",
                                            .symbolOccurence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstVariableDeclaratorDecl,
                                                },
                                            .isDeclaration = true,
                                        });
        checkSymbol("global.use.begin", SymbolQueryExpectedResult{
                                            .symbolType  = SymbolDeclType::GlobalVariable,
                                            .spelledText = "global",
                                            .symbolOccurence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstNameAccessExpr,
                                                },
                                        });
        checkSymbol("local.use.begin", SymbolQueryExpectedResult{
                                           .symbolType  = SymbolDeclType::LocalVariable,
                                           .spelledText = "local",
                                           .symbolOccurence =
                                               AstSymbolExpectedResult{
                                                   .tag = AstNodeTag::AstNameAccessExpr,
                                               },
                                       });
        checkSymbol("unknown.use.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::GlobalVariable,
                                             .spelledText = "unknown",
                                             .symbolOccurence =
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
                                            .symbolType  = SymbolDeclType::Parameter,
                                            .spelledText = "param",
                                            .symbolOccurence =
                                                AstSymbolExpectedResult{
                                                    .tag = AstNodeTag::AstParamDecl,
                                                },
                                            .isDeclaration = true,
                                        });
        checkSymbol("param.use.begin", SymbolQueryExpectedResult{
                                           .symbolType  = SymbolDeclType::Parameter,
                                           .spelledText = "param",
                                           .symbolOccurence =
                                               AstSymbolExpectedResult{
                                                   .tag = AstNodeTag::AstNameAccessExpr,
                                               },
                                       });
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            void ^[foo.decl.begin]foo()^[foo.decl.end];

            void bar()
            {
                ^[foo.use.begin]foo()^[foo.use.end];
                ^[unknown.use.begin]unknown()^[unknown.use.end];
            }

            void ^[foo.decl.def.begin]foo()^[foo.decl.def.end]
            {
            }
        )");

        checkSymbol("foo.decl.begin", SymbolQueryExpectedResult{
                                          .symbolType  = SymbolDeclType::Function,
                                          .spelledText = "foo",
                                          .symbolOccurence =
                                              AstSymbolExpectedResult{
                                                  .tag = AstNodeTag::AstFunctionDecl,
                                              },
                                          .isDeclaration = true,
                                      });
        checkSymbol("foo.use.begin", SymbolQueryExpectedResult{
                                         .symbolType  = SymbolDeclType::Function,
                                         .spelledText = "foo",
                                         .symbolOccurence =
                                             AstSymbolExpectedResult{
                                                 .tag = AstNodeTag::AstFunctionCallExpr,
                                             },
                                     });
        checkSymbol("unknown.use.begin", SymbolQueryExpectedResult{
                                             .symbolType  = SymbolDeclType::Function,
                                             .spelledText = "unknown",
                                             .symbolOccurence =
                                                 AstSymbolExpectedResult{
                                                     .tag = AstNodeTag::AstFunctionCallExpr,
                                                 },
                                         });
        checkSymbol("foo.decl.def.begin", SymbolQueryExpectedResult{
                                              .symbolType  = SymbolDeclType::Function,
                                              .spelledText = "foo",
                                              .symbolOccurence =
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
                                              .symbolType  = SymbolDeclType::Swizzle,
                                              .spelledText = "x",
                                              .symbolOccurence =
                                                  AstSymbolExpectedResult{
                                                      .tag = AstNodeTag::AstSwizzleAccessExpr,
                                                  },
                                          });
        checkSymbol("swizzle2.use.begin", SymbolQueryExpectedResult{
                                              .symbolType  = SymbolDeclType::Swizzle,
                                              .spelledText = "xyzw",
                                              .symbolOccurence =
                                                  AstSymbolExpectedResult{
                                                      .tag = AstNodeTag::AstSwizzleAccessExpr,
                                                  },
                                          });
    }
}