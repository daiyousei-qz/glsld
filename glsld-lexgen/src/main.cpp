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

    // Keywords
    // for (auto [klass, keyword] : GetAllKeywords()) {
    //     builder.AddTokenRegex(static_cast<int>(klass), RegexTextSeq{keyword});
    // }

    // Punctuations
    for (auto [klass, punct] : GetAllPunctuations()) {
        builder.AddTokenRegex(static_cast<int>(klass), RegexTextSeq{punct});
    }

    return builder.Build();
}

// FIXME: this is based on switch table. Bad for branch prediction?
auto GenerateLexSource(const NfaAutomata& automata) -> void
{
    fmt::print("#include <string_view>\n\n");
    fmt::print("// clang-format off\n");

    fmt::print("struct TokenizeResult {{\n");
    fmt::print("    int acceptedKlass;\n");
    fmt::print("    int numAcceptedChar;\n");
    fmt::print("}};\n");

    fmt::print("auto Tokenize(std::string_view remainingSource) -> TokenizeResult {{\n");
    fmt::print("    int index = 0;\n");
    fmt::print("    int numAcceptedChar = 0;\n");
    fmt::print("    int acceptedKlass = -1;\n");

    for (int i = 0; i < automata.NumState(); ++i) {
        auto state = automata[i];

        fmt::print("LexState_{}:\n", i);

        if (state->GetAcceptId() != -1) {
            fmt::print("// Accepting as {}\n", TokenKlassToString(static_cast<TokenKlass>(state->GetAcceptId())));
            fmt::print("numAcceptedChar = index;\n");
            fmt::print("acceptedKlass = {};\n", state->GetAcceptId());
        }

        auto outgoingTransitions = state->GetTransition();
        if (outgoingTransitions.empty()) {
            fmt::print("goto FinishToken;\n");
        }
        else {
            fmt::print("if (index == remainingSource.size()) {{\n");
            fmt::print("    goto FinishToken;\n");
            fmt::print("}}\n");

            fmt::print("switch(remainingSource[index++]) {{\n");
            for (auto transition : automata[i]->GetTransition()) {
                fmt::print("    case '{}': goto LexState_{};\n", static_cast<char>(transition.first),
                           transition.second->GetIndex());
            }
            fmt::print("    default: goto FinishToken;\n");
            fmt::print("}}\n");
        }
        fmt::print("\n");
    }

    fmt::print("FinishToken:\n");
    fmt::print("return {{acceptedKlass, numAcceptedChar}};\n");

    fmt::print("}}\n");
    fmt::print("// clang-format on\n");
}

int main()
{
    GenerateLexSource(CreateLexingAutomata());
    return 0;
}