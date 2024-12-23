#pragma once

#include "Basic/SourceInfo.h"
#include "Basic/StringMap.h"
#include "Compiler/SourceScanner.h"

#include "catch2/catch_all.hpp"

namespace glsld
{
    struct LabelledSource
    {
        std::string sourceText;
        StringMap<TextPosition> labels;
    };

    // Parses and remove "^label^" from the source text provided
    inline auto ParseLabelledSource(StringView sourceText) -> LabelledSource
    {
        SourceScanner scanner(sourceText, true);

        StringMap<TextPosition> labels;
        std::vector<char> sourceBuffer;
        std::vector<char> labelBuffer;
        while (!scanner.CursorAtEnd()) {
            auto textPosition = scanner.GetTextPosition();
            if (scanner.TryConsumeAsciiChar('^')) {
                while (!scanner.CursorAtEnd()) {
                    if (scanner.TryConsumeAsciiChar('^')) {
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
        }

        return LabelledSource{
            .sourceText = std::string(sourceBuffer.begin(), sourceBuffer.end()),
            .labels     = std::move(labels),
        };
    }
} // namespace glsld