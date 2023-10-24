#pragma once
#include "Compiler/SyntaxToken.h"
#include "Compiler/PreprocessContext.h"

namespace glsld
{
    // Flexible, could either
    // - Register token to LexContext
    // - Add token to a buffer
    class EmptyMacroExpansionCallback
    {
    public:
        auto OnYieldToken(const PPToken& token) -> void
        {
        }

        auto OnEnterMacroExpansion(const PPToken& macroUse) -> void
        {
        }

        auto OnExitMacroExpansion(const PPToken& macroUse) -> void
        {
        }
    };

    namespace detail
    {
        template <typename BaseCallback>
        class MacroArgumentExpansionCallback final : public EmptyMacroExpansionCallback
        {
        public:
            MacroArgumentExpansionCallback(BaseCallback& baseCallback, std::vector<PPToken>& outputBuffer)
                : baseCallback(baseCallback), outputBuffer(outputBuffer)
            {
            }

            using BaseCallbackType = BaseCallback;

            auto GetBaseCallback() -> BaseCallback&
            {
                return baseCallback;
            }

            auto OnYieldToken(const PPToken& token) -> void
            {
                outputBuffer.push_back(token);
            }

            auto OnEnterMacroExpansion(const PPToken& macroUse) -> void
            {
                baseCallback.OnEnterMacroExpansion(macroUse);
            }

            auto OnExitMacroExpansion(const PPToken& macroUse) -> void
            {
                baseCallback.OnExitMacroExpansion(macroUse);
            }

        private:
            BaseCallback& baseCallback;
            std::vector<PPToken>& outputBuffer;
        };

        // This class implement a recursive feedback-driven macro expansion. Tokens that are not eligible for
        // further expansion will be yielded to the callback.
        template <typename Callback, bool ArgExpansionMode = false>
        class MacroExpansionProcessorImpl
        {
        public:
            MacroExpansionProcessorImpl(PreprocessContext& ppContext, Callback callback)
                : ppContext(ppContext), callback(callback)
            {
            }

            auto Feed(const PPToken& token) -> void
            {
                if (pendingInvokedMacro) {
                    // First, we try to see if previous token is held for function-like macro expansion.

                    if (argLParenCounter > 0) {
                        // We are in the argument list of a function-like macro.
                        // So try to close the list with ')' or continue collecting arguments.
                        if (token.klass == TokenKlass::RParen) {
                            argLParenCounter -= 1;
                            if (argLParenCounter == 0) {
                                // The argument list has been fully collected.
                                auto macroUseTok    = pendingInvokedMacroToken;
                                auto invokedMacro   = pendingInvokedMacro;
                                auto args           = std::move(argBuffer);
                                pendingInvokedMacro = nullptr;
                                argBuffer.clear();

                                ExpandFunctionLikeMacro(macroUseTok, *invokedMacro, std::move(args));
                                TryExitMacroExpansion(macroUseTok);
                            }
                            else {
                                // Push the token to the argument buffer until the argument list is closed.
                                argBuffer.push_back(token);
                            }
                        }
                        else {
                            // NOTE if we see '(' inside the argument list, we have to increment the paranthesis
                            // counter for balancing.
                            if (token.klass == TokenKlass::LParen) {
                                argLParenCounter += 1;
                            }
                            argBuffer.push_back(token);
                        }
                    }
                    else {
                        // We are looking for a '(' to start the argument list of a function-like macro.
                        if (token.klass == TokenKlass::LParen) {
                            TryExitMacroExpansionDelayed();

                            argLParenCounter += 1;
                            callback.OnEnterMacroExpansion(pendingInvokedMacroToken);
                        }
                        else {
                            RevokePendingMacroInvocation();
                            TryExitMacroExpansionDelayed();

                            callback.OnYieldToken(token);
                        }
                    }
                }
                else if (token.klass == TokenKlass::Identifier) {
                    // Second, we try to see if the given token is an identifier.
                    // Only identifier can start a macro expansion.

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
                            callback.OnEnterMacroExpansion(token);
                            ExpandObjectLikeMacro(token, *macroDefinition);
                            TryExitMacroExpansion(token);
                        }
                    }
                    else {
                        // TODO: Expand builtin macros.
                        callback.OnYieldToken(token);
                    }
                }
                else {
                    // Otherwise, we just yield the token.

                    callback.OnYieldToken(token);
                }
            }

            // The token stream has been fully consumed.
            auto Finalize() -> void
            {
                if (pendingInvokedMacro) {
                    if (argLParenCounter > 0) {
                        // FIXME: unterminated argument list
                    }
                    else {
                        RevokePendingMacroInvocation();
                        TryExitMacroExpansionDelayed();
                    }
                }
            }

        private:
            auto RevokePendingMacroInvocation() -> void
            {
                GLSLD_ASSERT(pendingInvokedMacro);
                pendingInvokedMacro = nullptr;
                callback.OnYieldToken(pendingInvokedMacroToken);
            }

            auto TryExitMacroExpansion(const PPToken& macroUseTok) -> void
            {
                if (pendingInvokedMacro) {
                    pendingExitMacroToken = macroUseTok;
                }
                else {
                    callback.OnExitMacroExpansion(macroUseTok);
                }
            }

            auto TryExitMacroExpansionDelayed() -> void
            {
                if (pendingExitMacroToken) {
                    callback.OnExitMacroExpansion(*pendingExitMacroToken);
                    pendingExitMacroToken = std::nullopt;
                }
            }

            auto ExpandObjectLikeMacro(const PPToken& macroUseTok, MacroDefinition& macroDefinition) -> void
            {
                // Disable this macro to avoid recursive expansion during rescan.
                macroDefinition.Disable();

                // Replay the expanded tokens.
                for (PPToken token : macroDefinition.GetExpansionTokens()) {
                    // NOTE we assume that all tokens are expanded into the beginning of the macro use token.
                    token.spelledFile  = macroUseTok.spelledFile;
                    token.spelledRange = TextRange{macroUseTok.spelledRange.start};
                    Feed(token);
                }

                // Re-enable this macro.
                macroDefinition.Enable();
            }

            auto ExpandFunctionLikeMacro(const PPToken& macroUseTok, MacroDefinition& macroDefinition,
                                         std::vector<PPToken> args) -> void
            {
                const auto& paramTokens     = macroDefinition.GetParamTokens();
                const auto& expansionTokens = macroDefinition.GetExpansionTokens();

                // Collect the arguments
                std::vector<ArrayView<PPToken>> originalArgs;
                size_t beginIndex = 0;
                for (size_t endIndex = 0; endIndex < args.size(); ++endIndex) {
                    if (args[endIndex].klass == TokenKlass::Comma) {
                        originalArgs.push_back(ArrayView<PPToken>{args.data() + beginIndex, endIndex - beginIndex});
                        beginIndex = endIndex + 1;
                    }
                }

                if (beginIndex < args.size()) {
                    originalArgs.push_back(ArrayView<PPToken>{args.data() + beginIndex, args.size() - beginIndex});
                }

                if (originalArgs.size() != paramTokens.size()) {
                    // FIXME: Not exact number of arguments provided. Report error.
                    return;
                }

                // Expand the arguments completely, in case of substitution.
                std::vector<std::vector<PPToken>> expandedArgs;
                for (auto argView : originalArgs) {
                    // FIXME: Handle error in argument expansion.
                    if constexpr (ArgExpansionMode) {
                        using BaseCallback = typename Callback::BaseCallbackType;
                        MacroExpansionProcessorImpl<MacroArgumentExpansionCallback<BaseCallback>, true> argProcessor{
                            ppContext, MacroArgumentExpansionCallback<BaseCallback>{callback.GetBaseCallback(),
                                                                                    expandedArgs.emplace_back()}};

                        for (const PPToken& token : argView) {
                            argProcessor.Feed(token);
                        }
                    }
                    else {
                        using BaseCallback = Callback;
                        MacroExpansionProcessorImpl<MacroArgumentExpansionCallback<BaseCallback>, true> argProcessor{
                            ppContext,
                            MacroArgumentExpansionCallback<BaseCallback>{callback, expandedArgs.emplace_back()}};
                        for (const PPToken& token : argView) {
                            argProcessor.Feed(token);
                        }
                    }
                }

                // Disable this macro to avoid recursive expansion during rescan.
                macroDefinition.Disable();

                // Replay the expanded tokens
                for (PPToken token : macroDefinition.GetExpansionTokens()) {
                    // NOTE we assume that all tokens are expanded into the beginning of the macro use token.
                    token.spelledFile  = macroUseTok.spelledFile;
                    token.spelledRange = TextRange{macroUseTok.spelledRange.start};

                    // TODO: Handle token pasting
                    if (token.klass == TokenKlass::Identifier) {
                        bool substituted = false;
                        for (size_t i = 0; i < paramTokens.size(); ++i) {
                            if (token.text == paramTokens[i].text) {
                                for (const PPToken& argToken : expandedArgs[i]) {
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

            std::vector<PPToken> argBuffer;

            int argLParenCounter                 = 0;
            MacroDefinition* pendingInvokedMacro = nullptr;
            PPToken pendingInvokedMacroToken;
            std::optional<PPToken> pendingExitMacroToken;
        };
    } // namespace detail

    template <typename Callback>
    using MacroExpansionProcessor = detail::MacroExpansionProcessorImpl<Callback, false>;
} // namespace glsld