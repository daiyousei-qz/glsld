#pragma once

#include "Basic/SourceInfo.h"
#include "Basic/StringMap.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/SourceScanner.h"

#include "LanguageQueryProvider.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

namespace glsld
{
    struct LabelledSource
    {
        std::string sourceText;
        StringMap<TextPosition> labels;
    };

    class ServerTestContext
    {
    private:
        std::unique_ptr<LanguageQueryProvider> provider;
        StringMap<TextPosition> labels;

    public:
        ServerTestContext(std::unique_ptr<CompilerResult> result, StringMap<TextPosition> labels)
            : provider(std::make_unique<LanguageQueryProvider>(std::move(result), PreprocessInfoCache())),
              labels(std::move(labels))
        {
        }

        auto GetProvider() const -> const LanguageQueryProvider&
        {
            return *provider;
        }

        auto GetPosition(StringView label) const -> TextPosition
        {
            if (auto it = labels.Find(label); it != labels.end()) {
                return it->second;
            }
            else {
                UNSCOPED_INFO("Label not found: " + std::string(label));
                return TextPosition{};
            }
        }
        auto GetRange(StringView labelBegin, StringView lebelEnd) const -> TextRange
        {
            return TextRange{GetPosition(labelBegin), GetPosition(lebelEnd)};
        }
    };

    class ServerTestFixture
    {
    private:
        // Parses and remove "^[label]" from the source text provided
        auto ParseLabelledSource(StringView sourceText) const -> std::tuple<std::string, StringMap<TextPosition>>
        {
            SourceScanner scanner(sourceText, true);

            StringMap<TextPosition> labels;
            std::vector<char> sourceBuffer;
            std::vector<char> labelBuffer;
            while (!scanner.CursorAtEnd()) {
                auto textPosition = scanner.GetTextPosition();
                if (scanner.TryConsumeAsciiChar('^')) {
                    if (!scanner.TryConsumeAsciiChar('[')) {
                        UNSCOPED_INFO("Bad labelled source");
                    }
                    while (!scanner.CursorAtEnd()) {
                        if (scanner.TryConsumeAsciiChar(']')) {
                            // Found the end of the label
                            scanner.SetTextPosition(textPosition);
                            break;
                        }
                        else {
                            scanner.ConsumeChar(labelBuffer);
                        }
                    }
                }
                else {
                    scanner.ConsumeChar(sourceBuffer);
                }

                labels.Insert({std::string(labelBuffer.begin(), labelBuffer.end()), textPosition});
                labelBuffer.clear();
            }

            return {std::string(sourceBuffer.begin(), sourceBuffer.end()), std::move(labels)};
        }

    public:
        auto CompileLabelledSource(StringView labelledSourceText) const -> ServerTestContext
        {
            auto [sourceText, labels] = ParseLabelledSource(labelledSourceText);
            auto compiler             = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);

            auto result = compiler->CompileMainFile(nullptr, CompileMode::ParseOnly);
            return ServerTestContext(std::move(result), std::move(labels));
        }
    };
} // namespace glsld