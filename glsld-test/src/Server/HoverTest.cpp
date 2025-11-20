#include "ServerTestFixture.h"

#include "Feature/Hover.h"
#include "Support/SourceText.h"

#include <optional>

using namespace glsld;

static auto MockHover(const ServerTestFixture& fixture, TextPosition pos, const HoverConfig& config = {.enable = true})
    -> std::optional<lsp::Hover>
{
    return HandleHover(config, fixture.GetLanguageQueryInfo(),
                       lsp::HoverParams{
                           .textDocument = {"MockDocument"},
                           .position     = ToLspPosition(pos),
                       });
}

TEST_CASE_METHOD(ServerTestFixture, "HoverTest")
{
    auto checkNoHover = [this](StringView labelPos, const HoverConfig& config = {.enable = true}) {
        auto hover = MockHover(*this, GetLabelledPosition(labelPos), config);
        REQUIRE(!hover.has_value());
    };

    auto checkHover = [this](StringView labelPos, const HoverContent& expectedHover,
                             const HoverConfig& config = {.enable = true}) {
        auto hover = MockHover(*this, GetLabelledPosition(labelPos), config);
        REQUIRE(hover.has_value());
        REQUIRE(hover->contents.GetValue() == ComputeHoverText(expectedHover));
    };

    SECTION("Config")
    {
        CompileLabelledSource(R"(
            int ^[x.decl.pos]x = 1;
        )");

        checkNoHover("x.decl.pos", HoverConfig{.enable = false});
    }

    SECTION("Header")
    {
        CompileLabelledSource(R"(
            #include ^[header1.begin]<header1.h>^[header1.end]
            #include ^[header2.begin]"header2.h"^[header2.end]
        )");

        checkHover("header1.begin", HoverContent{
                                        .type        = SymbolDeclType::HeaderName,
                                        .name        = "<header1.h>",
                                        .description = "See ``",
                                        .range       = GetLabelledRange("header1.begin", "header1.end"),
                                    });
        checkHover("header2.begin", HoverContent{
                                        .type        = SymbolDeclType::HeaderName,
                                        .name        = "\"header2.h\"",
                                        .description = "See ``",
                                        .range       = GetLabelledRange("header2.begin", "header2.end"),
                                    });
    }

    // TODO: function-like macro
    SECTION("Macro")
    {
        CompileLabelledSource(R"(
            #ifdef ^[MACRO.unknown.use.begin]MACRO^[MACRO.unknown.use.end]
            #endif

            #define ^[MACRO.def.begin]MACRO^[MACRO.def.end] 1

            #ifdef ^[MACRO.use1.begin]MACRO^[MACRO.use1.end]
            #endif

            #undef ^[MACRO.use2.begin]MACRO^[MACRO.use2.end]

            #define ^[MACRO.redef.begin]MACRO^[MACRO.redef.end] 2

            #if defined ^[MACRO.use3.begin]MACRO^[MACRO.use3.end] && ^[MACRO.use4.begin]MACRO^[MACRO.use4.end]
            #endif

            int x = ^[MACRO.use5.begin]MACRO^[MACRO.use5.end];
        )");

        checkHover("MACRO.unknown.use.begin",
                   HoverContent{
                       .type    = SymbolDeclType::Macro,
                       .name    = "MACRO",
                       .range   = GetLabelledRange("MACRO.unknown.use.begin", "MACRO.unknown.use.end"),
                       .unknown = true,
                   });

        checkHover("MACRO.def.begin", HoverContent{
                                          .type    = SymbolDeclType::Macro,
                                          .name    = "MACRO",
                                          .code    = "#define MACRO 1",
                                          .range   = GetLabelledRange("MACRO.def.begin", "MACRO.def.end"),
                                          .unknown = false,
                                      });
        checkHover("MACRO.use1.begin", HoverContent{
                                           .type    = SymbolDeclType::Macro,
                                           .name    = "MACRO",
                                           .code    = "#define MACRO 1",
                                           .range   = GetLabelledRange("MACRO.use1.begin", "MACRO.use1.end"),
                                           .unknown = false,
                                       });
        checkHover("MACRO.use2.begin", HoverContent{
                                           .type    = SymbolDeclType::Macro,
                                           .name    = "MACRO",
                                           .code    = "#define MACRO 1",
                                           .range   = GetLabelledRange("MACRO.use2.begin", "MACRO.use2.end"),
                                           .unknown = false,
                                       });
        checkHover("MACRO.redef.begin", HoverContent{
                                            .type    = SymbolDeclType::Macro,
                                            .name    = "MACRO",
                                            .code    = "#define MACRO 2",
                                            .range   = GetLabelledRange("MACRO.redef.begin", "MACRO.redef.end"),
                                            .unknown = false,
                                        });
        checkHover("MACRO.use3.begin", HoverContent{
                                           .type    = SymbolDeclType::Macro,
                                           .name    = "MACRO",
                                           .code    = "#define MACRO 2",
                                           .range   = GetLabelledRange("MACRO.use3.begin", "MACRO.use3.end"),
                                           .unknown = false,
                                       });
        checkHover("MACRO.use4.begin", HoverContent{
                                           .type    = SymbolDeclType::Macro,
                                           .name    = "MACRO",
                                           .code    = "#define MACRO 2",
                                           .range   = GetLabelledRange("MACRO.use4.begin", "MACRO.use4.end"),
                                           .unknown = false,
                                       });
        checkHover("MACRO.use5.begin", HoverContent{
                                           .type    = SymbolDeclType::Macro,
                                           .name    = "MACRO",
                                           .code    = "#define MACRO 2",
                                           .range   = GetLabelledRange("MACRO.use5.begin", "MACRO.use5.end"),
                                           .unknown = false,
                                       });
    }

    SECTION("LayoutQualifier")
    {
        CompileLabelledSource(R"(
            layout(^[layout.qual.begin]location^[layout.qual.end] = 0) in vec4 position;
        )");

        checkHover("layout.qual.begin", HoverContent{
                                            .type    = SymbolDeclType::LayoutQualifier,
                                            .name    = "location",
                                            .range   = GetLabelledRange("layout.qual.begin", "layout.qual.end"),
                                            .unknown = false,
                                        });
    }

    SECTION("Struct")
    {
        CompileLabelledSource(R"(
            struct ^[struct.decl.begin]S^[struct.decl.end]
            {
            };

            void foo()
            {
                ^[struct.use.begin]S^[struct.use.end] s;
                ^[unknown.use.begin]unknown^[unknown.use.end] u;
            }
        )");

        checkHover("struct.decl.begin", HoverContent{
                                            .type    = SymbolDeclType::Type,
                                            .name    = "S",
                                            .code    = "struct S {\n}\n",
                                            .range   = GetLabelledRange("struct.decl.begin", "struct.decl.end"),
                                            .unknown = false,
                                        });
        checkHover("struct.use.begin", HoverContent{
                                           .type    = SymbolDeclType::Type,
                                           .name    = "S",
                                           .code    = "struct S {\n}\n",
                                           .range   = GetLabelledRange("struct.use.begin", "struct.use.end"),
                                           .unknown = false,
                                       });
        checkHover("unknown.use.begin", HoverContent{
                                            .type    = SymbolDeclType::Type,
                                            .name    = "unknown",
                                            .range   = GetLabelledRange("unknown.use.begin", "unknown.use.end"),
                                            .unknown = true,
                                        });
    }

    SECTION("StructMember")
    {
        CompileLabelledSource(R"(
            struct S
            {
                int ^[member.decl.begin]member^[member.decl.end];
            };

            void foo() {
                S s;
                s.^[member.use.begin]member^[member.use.end] = 1;
                s.^[unknown.use.begin]unknown^[unknown.use.end] = 2;
            }
        )");

        checkHover("member.decl.begin", HoverContent{
                                            .type       = SymbolDeclType::StructMember,
                                            .name       = "member",
                                            .symbolType = "int",
                                            .code       = "int member",
                                            .range      = GetLabelledRange("member.decl.begin", "member.decl.end"),
                                        });
        checkHover("member.use.begin", HoverContent{
                                           .type       = SymbolDeclType::StructMember,
                                           .name       = "member",
                                           .symbolType = "int",
                                           .code       = "int member",
                                           .range      = GetLabelledRange("member.use.begin", "member.use.end"),
                                       });
        checkHover("unknown.use.begin", HoverContent{
                                            .type       = SymbolDeclType::StructMember,
                                            .name       = "unknown",
                                            .symbolType = "__ErrorType",
                                            .range      = GetLabelledRange("unknown.use.begin", "unknown.use.end"),
                                            .unknown    = true,
                                        });
    }

    SECTION("Function")
    {
        CompileLabelledSource(R"(
            void ^[foo.decl.begin]foo^[foo.decl.end](int x)
            {
            }

            void main()
            {
                ^[foo.use.begin]foo^[foo.use.end](41);
                ^[unknown.use.begin]unknown^[unknown.use.end]();
            }
        )");

        checkHover("foo.decl.begin", HoverContent{
                                         .type       = SymbolDeclType::Function,
                                         .name       = "foo",
                                         .symbolType = "void",
                                         .parameters = {"int x"},
                                         .code       = "void foo(int x)",
                                         .range      = GetLabelledRange("foo.decl.begin", "foo.decl.end"),
                                         .unknown    = false,
                                     });
        checkHover("foo.use.begin", HoverContent{
                                        .type       = SymbolDeclType::Function,
                                        .name       = "foo",
                                        .symbolType = "void",
                                        .parameters = {"int x"},
                                        .code       = "void foo(int x)",
                                        .range      = GetLabelledRange("foo.use.begin", "foo.use.end"),
                                        .unknown    = false,
                                    });
        checkHover("unknown.use.begin", HoverContent{
                                            .type       = SymbolDeclType::Function,
                                            .name       = "unknown",
                                            .symbolType = "__ErrorType",
                                            .range      = GetLabelledRange("unknown.use.begin", "unknown.use.end"),
                                            .unknown    = true,
                                        });
    }

    SECTION("BlockName")
    {
        CompileLabelledSource(R"(
            uniform ^[ubo.decl.begin]UBO^[ubo.decl.end]
            {
                int x;
            };
            void foo() {
                // UBO doesn't introduce a type name in the symbol table, so this is a unknown type.
                ^[ubo.invalid.use.begin]UBO^[ubo.invalid.use.end] u;
            }
        )");

        checkHover("ubo.decl.begin", HoverContent{
                                         .type       = SymbolDeclType::Block,
                                         .name       = "UBO",
                                         .symbolType = "UBO",
                                         .code       = "uniform UBO {\n    int x;\n}\n",
                                         .range      = GetLabelledRange("ubo.decl.begin", "ubo.decl.end"),
                                         .unknown    = false,
                                     });
        checkHover("ubo.invalid.use.begin",
                   HoverContent{
                       .type    = SymbolDeclType::Type,
                       .name    = "UBO",
                       .range   = GetLabelledRange("ubo.invalid.use.begin", "ubo.invalid.use.end"),
                       .unknown = true,
                   });
    }

    SECTION("BlockMember")
    {
        CompileLabelledSource(R"(
            uniform UBO {
                int ^[ubo.x.decl.begin]x^[ubo.x.decl.end];
            };

            buffer SSBO {
                float ^[ssbo.y.decl.begin]y^[ssbo.y.decl.end];
            } ssbo;
            
            void foo() {
                ssbo.^[ssbo.y.use.begin]y^[ssbo.y.use.end] = ^[ubo.x.use.begin]x^[ubo.x.use.end];
            }
        )");

        checkHover("ubo.x.decl.begin", HoverContent{
                                           .type       = SymbolDeclType::BlockMember,
                                           .name       = "x",
                                           .symbolType = "int",
                                           .code       = "int x",
                                           .range      = GetLabelledRange("ubo.x.decl.begin", "ubo.x.decl.end"),
                                       });
        checkHover("ssbo.y.decl.begin", HoverContent{
                                            .type       = SymbolDeclType::BlockMember,
                                            .name       = "y",
                                            .symbolType = "float",
                                            .code       = "float y",
                                            .range      = GetLabelledRange("ssbo.y.decl.begin", "ssbo.y.decl.end"),
                                        });
        checkHover("ssbo.y.use.begin", HoverContent{
                                           .type       = SymbolDeclType::BlockMember,
                                           .name       = "y",
                                           .symbolType = "float",
                                           .code       = "float y",
                                           .range      = GetLabelledRange("ssbo.y.use.begin", "ssbo.y.use.end"),
                                       });
        checkHover("ubo.x.use.begin", HoverContent{
                                          .type       = SymbolDeclType::BlockMember,
                                          .name       = "x",
                                          .symbolType = "int",
                                          .code       = "int x",
                                          .range      = GetLabelledRange("ubo.x.use.begin", "ubo.x.use.end"),
                                      });
    }

    SECTION("BlockInstance")
    {
        CompileLabelledSource(R"(
            uniform UBO {
                int x;
            } ^[ubo.instance.decl.begin]uboInstance^[ubo.instance.decl.end];

            int foo() {
                return ^[ubo.instance.use.begin]uboInstance^[ubo.instance.use.end].x;
            }
        )");

        checkHover("ubo.instance.decl.begin",
                   HoverContent{
                       .type       = SymbolDeclType::BlockInstance,
                       .name       = "uboInstance",
                       .symbolType = "UBO",
                       .code       = "uniform UBO {\n    int x;\n} uboInstance\n",
                       .range      = GetLabelledRange("ubo.instance.decl.begin", "ubo.instance.decl.end"),
                   });
        checkHover("ubo.instance.use.begin",
                   HoverContent{
                       .type       = SymbolDeclType::BlockInstance,
                       .name       = "uboInstance",
                       .symbolType = "UBO",
                       .code       = "uniform UBO {\n    int x;\n} uboInstance\n",
                       .range      = GetLabelledRange("ubo.instance.use.begin", "ubo.instance.use.end"),
                   });
    }

    SECTION("Variable")
    {
        CompileLabelledSource(R"(
            int ^[global.decl.begin]global^[global.decl.end];
            int foo(int ^[param.decl.begin]param^[param.decl.end])
            {
                int ^[local.decl.begin]local^[local.decl.end];

                return ^[local.use.begin]local^[local.use.end]
                    + ^[param.use.begin]param^[param.use.end]
                    + ^[global.use.begin]global^[global.use.end]
                    + ^[unknown.use.begin]unknown^[unknown.use.end];
            }
        )");

        checkHover("global.decl.begin", HoverContent{
                                            .type       = SymbolDeclType::GlobalVariable,
                                            .name       = "global",
                                            .symbolType = "int",
                                            .code       = "int global",
                                            .range      = GetLabelledRange("global.decl.begin", "global.decl.end"),
                                            .unknown    = false,
                                        });
        checkHover("param.decl.begin", HoverContent{
                                           .type       = SymbolDeclType::Parameter,
                                           .name       = "param",
                                           .symbolType = "int",
                                           .code       = "int param",
                                           .range      = GetLabelledRange("param.decl.begin", "param.decl.end"),
                                           .unknown    = false,
                                       });
        checkHover("local.decl.begin", HoverContent{
                                           .type       = SymbolDeclType::LocalVariable,
                                           .name       = "local",
                                           .symbolType = "int",
                                           .code       = "int local",
                                           .range      = GetLabelledRange("local.decl.begin", "local.decl.end"),
                                           .unknown    = false,
                                       });

        checkHover("local.use.begin", HoverContent{
                                          .type       = SymbolDeclType::LocalVariable,
                                          .name       = "local",
                                          .symbolType = "int",
                                          .code       = "int local",
                                          .range      = GetLabelledRange("local.use.begin", "local.use.end"),
                                          .unknown    = false,
                                      });
        checkHover("param.use.begin", HoverContent{
                                          .type       = SymbolDeclType::Parameter,
                                          .name       = "param",
                                          .symbolType = "int",
                                          .code       = "int param",
                                          .range      = GetLabelledRange("param.use.begin", "param.use.end"),
                                          .unknown    = false,
                                      });
        checkHover("global.use.begin", HoverContent{
                                           .type       = SymbolDeclType::GlobalVariable,
                                           .name       = "global",
                                           .symbolType = "int",
                                           .code       = "int global",
                                           .range      = GetLabelledRange("global.use.begin", "global.use.end"),
                                           .unknown    = false,
                                       });
        checkHover("unknown.use.begin", HoverContent{
                                            .type       = SymbolDeclType::GlobalVariable,
                                            .name       = "unknown",
                                            .symbolType = "__ErrorType",
                                            .range      = GetLabelledRange("unknown.use.begin", "unknown.use.end"),
                                            .unknown    = true,
                                        });
    }

    SECTION("Swizzle")
    {
        CompileLabelledSource(R"(
            vec4 foo(vec4 v)
            {
                v.^[swizzle1.begin]x^[swizzle1.end] = v.y;
                return v.^[swizzle2.begin]xyz^[swizzle2.end];
            }
        )");

        checkHover("swizzle1.begin", HoverContent{
                                         .type       = SymbolDeclType::Swizzle,
                                         .name       = "x",
                                         .symbolType = "float",
                                         .range      = GetLabelledRange("swizzle1.begin", "swizzle1.end"),
                                         .unknown    = false,
                                     });
        checkHover("swizzle2.begin", HoverContent{
                                         .type       = SymbolDeclType::Swizzle,
                                         .name       = "xyz",
                                         .symbolType = "vec3",
                                         .range      = GetLabelledRange("swizzle2.begin", "swizzle2.end"),
                                         .unknown    = false,
                                     });
    }

    SECTION("ConstValue")
    {
        CompileLabelledSource(R"(
            const int ^[one.decl.begin]one^[one.decl.end] = 1;
            const vec3 ^[v.decl.begin]v^[v.decl.end] = vec3(1.0, 2.0, 3.0);
            const int[2] ^[arr.decl.begin]arr^[arr.decl.end] = {1, 2};
            const struct S { int a; float b; } ^[complex.decl.begin]complex^[complex.decl.end] = {1, 2.0};

            void foo()
            {
                v.^[v.swizzle.begin]xy^[v.swizzle.end];
            }
        )");

        checkHover("one.decl.begin", HoverContent{
                                         .type        = SymbolDeclType::GlobalVariable,
                                         .name        = "one",
                                         .symbolType  = "int",
                                         .symbolValue = "1",
                                         .code        = "const int one = ...",
                                         .range       = GetLabelledRange("one.decl.begin", "one.decl.end"),
                                     });

        checkHover("v.decl.begin", HoverContent{
                                       .type        = SymbolDeclType::GlobalVariable,
                                       .name        = "v",
                                       .symbolType  = "vec3",
                                       .symbolValue = "vec3(1, 2, 3)",
                                       .code        = "const vec3 v = ...",
                                       .range       = GetLabelledRange("v.decl.begin", "v.decl.end"),
                                   });

        // Note that we don't show constant value for aggregate types.
        checkHover("arr.decl.begin", HoverContent{
                                         .type       = SymbolDeclType::GlobalVariable,
                                         .name       = "arr",
                                         .symbolType = "int[2]",
                                         .code       = "const int[2] arr = ...",
                                         .range      = GetLabelledRange("arr.decl.begin", "arr.decl.end"),
                                     });

        // Note that we don't show constant value for aggregate types.
        checkHover("complex.decl.begin", HoverContent{
                                             .type       = SymbolDeclType::GlobalVariable,
                                             .name       = "complex",
                                             .symbolType = "S",
                                             .code       = "const struct S { ... } complex = ...",
                                             .range      = GetLabelledRange("complex.decl.begin", "complex.decl.end"),
                                         });

        checkHover("v.swizzle.begin", HoverContent{
                                          .type        = SymbolDeclType::Swizzle,
                                          .name        = "xy",
                                          .symbolType  = "vec2",
                                          .symbolValue = "vec2(1, 2)",
                                          .range       = GetLabelledRange("v.swizzle.begin", "v.swizzle.end"),
                                      });
        // TODO: add more tests
    }

    SECTION("CommentDescription")
    {
        CompileLabelledSource(R"(
            int ^[global.decl.begin]global^[global.decl.end]; // This is a global variable.

            // This is a function.
            void ^[foo.decl.begin]foo^[foo.decl.end]() {
            }

            // This is another variable.
            int ^[otherGlobal.decl.begin]otherGlobal^[otherGlobal.decl.end]; // And here's another comment.

            // This is a MACRO
            #define ^[MACRO.def.begin]MACRO^[MACRO.def.end] 1
            void ^[bar.def.begin]bar^[bar.def.end]() {
                // Notably, comment description for `MACRO` should not be attached to `bar`
            }
        )");

        // FIXME: enable after fixing comment extraction
        // checkHover("global.decl.begin", HoverContent{
        //                                     .type        = SymbolDeclType::GlobalVariable,
        //                                     .name        = "global",
        //                                     .exprType    = "int",
        //                                     .description = "This is a global variable.",
        //                                     .code        = "int global",
        //                                     .range       = GetLabelledRange("global.decl.begin",
        //                                     "global.decl.end"),
        //                                 });
        // checkHover("foo.decl.begin", HoverContent{
        //                                  .type        = SymbolDeclType::Function,
        //                                  .name        = "foo",
        //                                  .returnType  = "void",
        //                                  .parameters  = {},
        //                                  .description = "This is a function.",
        //                                  .code        = "void foo()",
        //                                  .range       = GetLabelledRange("foo.decl.begin", "foo.decl.end"),
        //                              });
        // checkHover("otherGlobal.decl.begin",
        //            HoverContent{
        //                .type        = SymbolDeclType::GlobalVariable,
        //                .name        = "otherGlobal",
        //                .exprType    = "int",
        //                .description = "And here's another comment.",
        //                .code        = "int otherGlobal",
        //                .range       = GetLabelledRange("otherGlobal.decl.begin", "otherGlobal.decl.end"),
        //            });
        // checkHover("MACRO.def.begin", HoverContent{
        //                                   .type        = SymbolDeclType::Macro,
        //                                   .name        = "MACRO",
        //                                   .description = "This is a MACRO",
        //                                   .code        = "#define MACRO 1",
        //                                   .range       = GetLabelledRange("MACRO.def.begin", "MACRO.def.end"),
        //                               });
        // checkHover("bar.def.begin", HoverContent{
        //                                 .type  = SymbolDeclType::Function,
        //                                 .name  = "bar",
        //                                 .code  = "void bar()",
        //                                 .range = GetLabelledRange("bar.def.begin", "bar.def.end"),
        //                             });
    }
}