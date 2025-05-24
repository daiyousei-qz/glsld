#pragma once

#include "Basic/SourceInfo.h"
#include "Basic/StringMap.h"
#include "Compiler/CompilerInvocation.h"
#include "Compiler/SourceScanner.h"
#include "Server/LanguageQueryInfo.h"

#include <catch2/catch_test_macros.hpp>

namespace glsld
{
    struct LabelledSource
    {
        std::string sourceText;
        StringMap<TextPosition> labels;
    };

    class ServerTestFixture
    {
    private:
        std::unique_ptr<LanguageQueryInfo> info = nullptr;
        StringMap<TextPosition> labels;

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
        auto GetLanguageQueryInfo() const -> const LanguageQueryInfo&
        {
            return *info;
        }

        auto GetLabelledPosition(StringView label) const -> TextPosition
        {
            if (auto it = labels.Find(label); it != labels.end()) {
                return it->second;
            }
            else {
                UNSCOPED_INFO("Label not found: " + std::string(label));
                return TextPosition{};
            }
        }
        auto GetLabelledRange(StringView labelBegin, StringView lebelEnd) const -> TextRange
        {
            return TextRange{GetLabelledPosition(labelBegin), GetLabelledPosition(lebelEnd)};
        }

        auto CompileLabelledSource(StringView labelledSourceText) -> void
        {
            auto [sourceText, labels] = ParseLabelledSource(labelledSourceText);
            auto ppInfoStore          = std::make_unique<PreprocessSymbolStore>();
            auto compiler             = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);

            auto ppCallback = ppInfoStore->GetCollectionCallback();
            auto result     = compiler->CompileMainFile(ppCallback.get(), CompileMode::ParseOnly);

            this->info   = std::make_unique<LanguageQueryInfo>(std::move(result), std::move(ppInfoStore));
            this->labels = std::move(labels);
        }
    };
} // namespace glsld