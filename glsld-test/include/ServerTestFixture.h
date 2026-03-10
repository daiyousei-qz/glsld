#pragma once

#include "StringMaker.h"

#include "Basic/SourceInfo.h"
#include "Support/StringMap.h"
#include "Compiler/CompilerInvocation.h"
#include "Server/LanguageQueryInfo.h"

#include <catch2/catch_message.hpp>
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
        auto ParseLabelledSource(SourceTextView labeledSourceText) const
            -> std::tuple<std::string, StringMap<TextPosition>>
        {

            StringMap<TextPosition> labels;
            std::string sourceBuffer;
            std::string labelBuffer;
            int currentLine      = 0;
            int currentCharacter = 0; // Here we should count in UTF-8 code units
            for (const char* p = labeledSourceText.begin(); p != labeledSourceText.end();) {
                if (p[0] == '^' && p[1] == '[') {
                    // We are seeing a label. Try to consume it.
                    p += 2; // Skip '^['
                    labelBuffer.clear();
                    while (true) {
                        if (p == labeledSourceText.end()) {
                            UNSCOPED_INFO(fmt::format("Unterminated label: {}", labelBuffer));
                            break;
                        }
                        else if (*p == ']') {
                            // Found the end of the label
                            labels.Insert({std::move(labelBuffer), TextPosition{currentLine, currentCharacter}});
                            ++p; // Skip ']'
                            break;
                        }
                        else {
                            // Part of the label
                            labelBuffer.push_back(*p);
                            ++p;
                        }
                    }
                }
                else {
                    if (*p == '\n') {
                        currentLine += 1;
                        currentCharacter = 0;
                    }
                    else {
                        currentCharacter += 1;
                    }
                    sourceBuffer.push_back(*p);
                    ++p;
                }
            }

            return {std::move(sourceBuffer), std::move(labels)};
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
        auto GetLabelledRange(StringView labelBegin, StringView labelEnd) const -> TextRange
        {
            return TextRange{GetLabelledPosition(labelBegin), GetLabelledPosition(labelEnd)};
        }

        auto CompileLabelledSource(SourceTextView labeledSourceText) -> void
        {
            auto [sourceText, labels] = ParseLabelledSource(labeledSourceText);
            auto ppInfoStore          = std::make_unique<PreprocessInfoStore>();
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