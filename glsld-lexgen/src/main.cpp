#include "LexerBuilder.h"
#include "SyntaxToken.h"

using namespace glsld;

auto CreateLexingAutomata() -> NfaAutomata
{
    LexerBuilder builder;

    int acceptId = 0;

    // Integer constant
    {
        auto regexOctSeq = RegexSeq{RegexTextSeq{"0"}, RegexRepeat{RegexTextChoice{"01234567"}}};
        auto regexDecSeq = RegexSeq{RegexTextRange{'1', '9'}, RegexRepeat{RegexTextChoice{"0123456789"}}};
        auto regexHexSeq =
            RegexSeq{RegexTextSeq{"0"}, RegexTextChoice{"xX"}, RegexRepeat{RegexTextChoice{"0123456789abcdefABCDEF"}}};

        auto regexIntSuffix = RegexChoice{RegexTextSeq{"u"}, RegexTextSeq{"U"}};
        auto regexInteger = RegexSeq{RegexChoice{regexOctSeq, regexDecSeq, regexHexSeq}, RegexOptional{regexIntSuffix}};

        builder.AddTokenRegex(static_cast<int>(TokenKlass::IntegerConstant), regexInteger);
    }

    // Float constant
    {
        auto regexDigitSeq  = RegexSeq{RegexTextRange{'0', '9'}, RegexRepeat{RegexTextRange{'0', '9'}}};
        auto regexFracConst = RegexChoice{
            RegexSeq{regexDigitSeq, RegexTextSeq{"."}, RegexOptional{regexDigitSeq}},
            RegexSeq{RegexTextSeq{"."}, regexDigitSeq},

        };
        auto regexExponentPart = RegexSeq{RegexTextChoice{"eE"}, RegexOptional{RegexTextChoice{"+-"}}, regexDigitSeq};
        auto regexFPSuffix = RegexChoice{RegexTextSeq{"f"}, RegexTextSeq{"F"}, RegexTextSeq{"lf"}, RegexTextSeq{"LF"}};

        auto regexFP = RegexSeq{RegexChoice{RegexSeq{regexFracConst, RegexOptional{regexExponentPart}},
                                            RegexSeq{regexDigitSeq, regexExponentPart}},
                                RegexOptional{regexFPSuffix}};

        builder.AddTokenRegex(static_cast<int>(TokenKlass::FloatConstant), regexFP);
    }

    // Identifier
    {
        auto regexIdentifierBegin = RegexChoice{RegexTextRange{'a', 'z'}, RegexTextRange{'A', 'Z'}, RegexTextSeq{"_"}};
        auto regexIdentifierContinue = RegexChoice{regexIdentifierBegin, RegexTextRange{'0', '9'}};

        auto regexIdentifier = RegexSeq{regexIdentifierBegin, RegexRepeat{regexIdentifierContinue}};
        builder.AddTokenRegex(static_cast<int>(TokenKlass::Identifier), regexIdentifier);
    }

    // Special Characters
    builder.AddTokenRegex(static_cast<int>(TokenKlass::Hash), RegexTextSeq{"#"});
    builder.AddTokenRegex(static_cast<int>(TokenKlass::HashHash), RegexTextSeq{"##"});

    // Punctuations
    for (auto [klass, punct] : GetAllPunctuations()) {
        builder.AddTokenRegex(static_cast<int>(klass), RegexTextSeq{punct});
    }

    // NOTE keywords are identified by post-processing identifier. This could help simplify the lexer state
    // machine by a lot.

    return builder.Build();
}

auto GenerateLexSource(const NfaAutomata& automata) -> void
{
    fmt::print("#pragma once\n");
    fmt::print("// clang-format off\n");
    fmt::print("#include <optional>\n");
    fmt::print("#include <vector>\n");
    fmt::print("#include \"SyntaxToken.h\"\n");
    fmt::print("#include \"SourceInfo.h\"\n\n");

    fmt::print("namespace glsld::detail {{\n");
    fmt::print("auto Tokenize(SourceScanner& srcView, std::vector<char>& buffer) -> TokenKlass {{\n");

    fmt::print("TokenKlass acceptedKlass;\n");
    fmt::print("size_t acceptedSize;\n");
    fmt::print("SourceScanner acceptedCheckpoint;\n");
    fmt::print("char ch;\n");

    for (int i = 0; i < automata.NumState(); ++i) {
        auto state = automata[i];

        fmt::print("LexState_{}:\n", i);
        if (state->GetAcceptId() != -1) {
            fmt::print("// Accepting as {}\n", TokenKlassToString(static_cast<TokenKlass>(state->GetAcceptId())));
            fmt::print("acceptedCheckpoint = srcView.Clone();\n");
            fmt::print("acceptedKlass = static_cast<TokenKlass>({});\n", state->GetAcceptId());
            fmt::print("acceptedSize = buffer.size();\n\n");
        }

        fmt::print("ch = srcView.ConsumeChar();\n");
        fmt::print("buffer.push_back(ch);\n\n");

        if (i == 0) {
            GLSLD_REQUIRE(state->GetAcceptId() == -1);
            fmt::print("// Initialize error token as a fallback\n");
            fmt::print("acceptedCheckpoint = srcView.Clone();\n");
            fmt::print("acceptedKlass = TokenKlass::Error;\n");
            fmt::print("acceptedSize = buffer.size();\n\n");
        }

        auto outgoingTransitions = state->GetTransition();
        if (outgoingTransitions.empty()) {
            fmt::print("goto FinishToken;\n");
        }
        else {
            fmt::print("switch(ch) {{\n");
            for (auto transition : automata[i]->GetTransition()) {
                GLSLD_REQUIRE(transition.second->GetIndex() != 0);
                fmt::print("    case '{}': goto LexState_{};\n", static_cast<char>(transition.first),
                           transition.second->GetIndex());
            }
            fmt::print("    default: goto FinishToken;\n");
            fmt::print("}}\n");
        }
        fmt::print("\n");
    }

    fmt::print("FinishToken:\n");
    fmt::print("srcView.Restore(acceptedCheckpoint);\n");
    fmt::print("buffer.resize(acceptedSize);\n");
    fmt::print("return acceptedKlass;\n");

    fmt::print("}}\n");
    fmt::print("}}\n");
    fmt::print("// clang-format on\n");
}

int main()
{
    GenerateLexSource(CreateLexingAutomata());
    return 0;
}