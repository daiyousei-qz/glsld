#pragma once
#include "SyntaxToken.h"
#include "PreprocessContext.h"

namespace glsld
{
    // Flexible, could either
    // - Register token to LexContext
    // - Add token to a buffer
    class ExpandToVoidCallback
    {
    public:
        auto OnYieldToken(const PPTokenData& token) -> void;
    };

    class ExpandToVectorCallback
    {
    public:
        ExpandToVectorCallback(std::vector<PPTokenData>& outputBuffer) : outputBuffer(outputBuffer)
        {
        }

        auto OnYieldToken(const PPTokenData& token) -> void
        {
            outputBuffer.push_back(token);
        }

    private:
        std::vector<PPTokenData>& outputBuffer;
    };

    // This class implement a recursive feedback-driven macro expansion. Tokens that are not eligible for
    // further expansion will be yielded to the callback.
    template <typename Callback>
    class MacroExpansionProcessor
    {
    public:
        MacroExpansionProcessor(PreprocessContext& ppContext, Callback callback)
            : ppContext(ppContext), callback(callback)
        {
        }

        auto Feed(const PPTokenData& token) -> void
        {
            if (token.klass == TokenKlass::Identifier) {
                auto macroDefinition = ppContext.FindEnabledMacroDefinition(token.text);
                if (macroDefinition) {
                    if (macroDefinition->IsFunctionLike()) {
                        // For function-like macro, we need to collect the arguments first, if any.
                        // So we store the macro definition in a pending state.
                        pendingInvokedMacroToken = token;
                        pendingInvokedMacro      = macroDefinition;
                    }
                    else {
                        // For object-like macro, we can expand it immediately.
                        ExpandObjectLikeMacro(*macroDefinition);
                    }
                }
                else {
                    HandleOtherIdentifier(token);
                }
            }
            else if (pendingInvokedMacro) {
                if (argLParenCounter > 0) {
                    if (token.klass == TokenKlass::RParen) {
                        argLParenCounter -= 1;
                        if (argLParenCounter == 0) {
                            // The argument list has been fully collected.
                            auto invokedMacro   = pendingInvokedMacro;
                            auto args           = std::move(argBuffer);
                            pendingInvokedMacro = nullptr;
                            argBuffer.clear();

                            ExpandFunctionLikeMacro(*invokedMacro, std::move(args));
                        }
                        else {
                            argBuffer.push_back(token);
                        }
                    }
                    else {
                        if (token.klass == TokenKlass::LParen) {
                            argLParenCounter += 1;
                        }
                        argBuffer.push_back(token);
                    }
                }
                else if (token.klass == TokenKlass::LParen) {
                    argLParenCounter += 1;
                }
                else {
                    pendingInvokedMacro = nullptr;
                    callback.OnYieldToken(pendingInvokedMacroToken);
                    callback.OnYieldToken(token);
                }
            }
            else {
                callback.OnYieldToken(token);
            }
        }

        // The token stream has been fully consumed.
        auto Finalize() -> void
        {
        }

    private:
        auto HandleOtherIdentifier(const PPTokenData& token) -> void
        {
            // TODO: Expand builtin macros.
            callback.OnYieldToken(token);
        }

        auto ExpandObjectLikeMacro(MacroDefinition& macroDefinition) -> void
        {
            // Disable this macro to avoid recursive expansion during rescan.
            macroDefinition.Disable();

            // Replay the expanded tokens.
            for (const PPTokenData& token : macroDefinition.GetExpansionTokens()) {
                Feed(token);
            }

            // Re-enable this macro.
            macroDefinition.Enable();
        }

        auto ExpandFunctionLikeMacro(MacroDefinition& macroDefinition, std::vector<PPTokenData> args) -> void
        {
            const auto& paramTokens     = macroDefinition.GetParamTokens();
            const auto& expansionTokens = macroDefinition.GetExpansionTokens();

            // Collect the arguments
            std::vector<ArrayView<PPTokenData>> originalArgs;
            size_t beginIndex = 0;
            for (size_t endIndex = 0; endIndex < args.size(); ++endIndex) {
                if (args[endIndex].klass == TokenKlass::Comma) {
                    originalArgs.push_back(ArrayView<PPTokenData>{args.data() + beginIndex, endIndex - beginIndex});
                    beginIndex = endIndex + 1;
                }
            }

            if (beginIndex < args.size()) {
                originalArgs.push_back(ArrayView<PPTokenData>{args.data() + beginIndex, args.size() - beginIndex});
            }

            if (originalArgs.size() != paramTokens.size()) {
                // FIXME: Not exact number of arguments provided. Report error.
                return;
            }

            // Expand the arguments completely, in case of substitution.
            std::vector<std::vector<PPTokenData>> expandedArgs;
            for (auto argView : originalArgs) {
                // FIXME: Handle error in argument expansion.
                MacroExpansionProcessor<ExpandToVectorCallback> argProcessor{
                    ppContext, ExpandToVectorCallback{expandedArgs.emplace_back()}};
                for (const PPTokenData& token : argView) {
                    argProcessor.Feed(token);
                }
            }

            // Disable this macro to avoid recursive expansion during rescan.
            macroDefinition.Disable();

            // Replay the expanded tokens
            for (const PPTokenData& token : macroDefinition.GetExpansionTokens()) {
                // TODO: Handle token pasting
                if (token.klass == TokenKlass::Identifier) {
                    bool substituted = false;
                    for (size_t i = 0; i < paramTokens.size(); ++i) {
                        if (token.text == paramTokens[i].text) {
                            for (const PPTokenData& argToken : expandedArgs[i]) {
                                Feed(argToken);
                            }
                            substituted = true;
                            break;
                        }
                    }

                    if (!substituted) {
                        Feed(token);
                    }
                }
                else {
                    Feed(token);
                }
            }

            // Re-enable this macro.
            macroDefinition.Enable();
        }

        PreprocessContext& ppContext;
        Callback callback;

        std::vector<PPTokenData> argBuffer;

        int argLParenCounter                 = 0;
        MacroDefinition* pendingInvokedMacro = nullptr;
        PPTokenData pendingInvokedMacroToken;
    };

} // namespace glsld