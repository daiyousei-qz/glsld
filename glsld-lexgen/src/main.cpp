#include "Basic/Print.h"
#include "Compiler/SyntaxToken.h"

#include "LexerBuilder.h"

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

auto GenerateLexSource(FILE* file, const NfaAutomata& automata) -> void
{
    auto printToFile = [file]<typename... Ts>(fmt::format_string<Ts...> format, Ts&&... args) constexpr {
        fmt::print(file, format, std::forward<Ts>(args)...);
    };

    printToFile("// clang-format off\n");
    printToFile("#include <optional>\n");
    printToFile("#include <vector>\n");
    printToFile("#include \"Compiler/SyntaxToken.h\"\n");
    printToFile("#include \"Compiler/SourceScanner.h\"\n\n");

    printToFile("namespace glsld::detail {{\n");
    printToFile("auto Tokenize(SourceScanner& srcView, std::vector<char>& buffer) -> TokenKlass {{\n");

    printToFile("// Initialize unknown token as a fallback\n");
    printToFile("TokenKlass acceptedKlass = TokenKlass::Unknown;\n");
    printToFile("size_t acceptedSize = 0;\n");
    printToFile("ScannerCheckPoint acceptedCheckpoint = srcView.CreateCheckPoint();\n");
    printToFile("char ch;\n");

    for (int i = 0; i < automata.NumState(); ++i) {
        auto state = automata[i];

        printToFile("LexState_{}:\n", i);
        if (state->GetAcceptId() != -1) {
            printToFile("// Accepting as {}\n", TokenKlassToString(static_cast<TokenKlass>(state->GetAcceptId())));
            printToFile("acceptedCheckpoint = srcView.CreateCheckPoint();\n");
            printToFile("acceptedKlass = static_cast<TokenKlass>({});\n", state->GetAcceptId());
            printToFile("acceptedSize = buffer.size();\n\n");
        }

        printToFile("ch = srcView.ConsumeAsciiChar();\n");
        printToFile("buffer.push_back(ch);\n\n");

        auto outgoingTransitions = state->GetTransition();
        if (outgoingTransitions.empty()) {
            printToFile("goto FinishToken;\n");
        }
        else {
            printToFile("switch(ch) {{\n");
            for (auto transition : automata[i]->GetTransition()) {
                GLSLD_REQUIRE(transition.second->GetIndex() != 0);
                printToFile("    case '{}': goto LexState_{};\n", static_cast<char>(transition.first),
                            transition.second->GetIndex());
            }
            printToFile("    default: goto FinishToken;\n");
            printToFile("}}\n");
        }
        printToFile("\n");
    }

    printToFile("FinishToken:\n");
    printToFile("srcView.RestoreCheckPoint(acceptedCheckpoint);\n");
    printToFile("buffer.resize(acceptedSize);\n");
    printToFile("return acceptedKlass;\n");

    printToFile("}}\n");
    printToFile("}}\n");
    printToFile("// clang-format on\n");
}

int main(int argc, const char** argv)
{
    if (argc != 2) {
        Print("Usage: {} <output file>\n", argv[0]);
        return -1;
    }

    auto file = fopen(argv[1], "wb");
    if (!file) {
        Print("Failed to open file {}\n", argv[1]);
        return -1;
    }

    GenerateLexSource(file, CreateLexingAutomata());
    return 0;
}