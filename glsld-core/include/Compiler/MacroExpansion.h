#pragma once
#include "Basic/AtomTable.h"
#include "Compiler/MacroTable.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/MacroDefinition.h"
#include "Compiler/PPTokenScanner.h"

namespace glsld
{
    auto TokenizeOnce(StringView text) -> std::tuple<TokenKlass, StringView, StringView>;

    // Flexible, could either
    // - Output token to token stream
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
        private:
            BaseCallback& baseCallback;
            std::vector<PPToken>& outputBuffer;

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
        };

        // This class implement a recursive feedback-driven macro expansion. Tokens that are not eligible for
        // further expansion will be yielded to the callback.
        template <typename Callback, bool ArgExpansionMode = false>
        class MacroExpansionProcessorImpl
        {
        private:
            AtomTable& atomTable;
            MacroTable& macroTable;
            Callback callback;

            std::vector<PPToken> argBuffer;

            int argLParenCounter                 = 0;
            MacroDefinition* pendingInvokedMacro = nullptr;
            PPToken pendingInvokedMacroToken;
            std::optional<PPToken> pendingExitMacroToken;

        public:
            MacroExpansionProcessorImpl(AtomTable& atomTable, MacroTable& macroTable, Callback callback)
                : atomTable(atomTable), macroTable(macroTable), callback(callback)
            {
            }

            // Feed a token to the processor for macro expansion.
            auto Feed(const PPToken& token) -> void
            {
                if (pendingInvokedMacro) {
                    // First, we try to see if previous token is witheld for potential function-like macro expansion.
                    if (argLParenCounter == 0) {
                        // We are looking for a '(' to start the argument list of a function-like macro.
                        if (token.klass == TokenKlass::LParen) {
                            TryExitMacroExpansionDelayed();

                            argLParenCounter += 1;
                            callback.OnEnterMacroExpansion(pendingInvokedMacroToken);
                        }
                        else {
                            // This is not a function-like macro invocation.
                            RevokePendingMacroInvocation();
                            TryExitMacroExpansionDelayed();

                            callback.OnYieldToken(token);
                        }
                    }
                    else {
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
                                // This is not the matching ')' of the argument list. Treat it as regular token.
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
                }
                else if (token.klass == TokenKlass::Identifier) {
                    // Second, we try to see if the given token is an identifier.
                    // Only identifier can start a macro expansion.
                    auto macroDefinition = macroTable.FindEnabledMacroDefinition(token.text);
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
                        // FIXME: Expand builtin macros.
                        callback.OnYieldToken(token);
                    }
                }
                else {
                    // Otherwise, we just yield the token.
                    callback.OnYieldToken(token);
                }
            }

            // The token stream has been fully consumed, finalize the macro expansion and clear unfinished expansion.
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

            auto FeedPastedToken(const PPToken& macroUseTok, const PPToken& lhs, const PPToken& rhs) -> void
            {
                std::string pastedText;
                pastedText += lhs.text.StrView();
                pastedText += rhs.text.StrView();

                auto [klass, tokText, remText] = TokenizeOnce(pastedText);
                if (remText.Empty()) {
                    Feed(PPToken{
                        .klass                = klass,
                        .spelledFile          = macroUseTok.spelledFile,
                        .spelledRange         = TextRange{macroUseTok.spelledRange.start},
                        .text                 = atomTable.GetAtom(tokText),
                        .isFirstTokenOfLine   = false,
                        .hasLeadingWhitespace = false,
                    });
                }
                else {
                    // FIXME: report error, bad token pasting
                }
            }

            auto FeedMacroExpansion(const PPToken& macroUseTok, const MacroDefinition& macroDefinition,
                                    ArrayView<ArrayView<PPToken>> originalArgs,
                                    ArrayView<std::vector<PPToken>> expandedArgs) -> void
            {
                const auto& paramTokens = macroDefinition.GetParamTokens();

                PPTokenScanner macroScanner{macroDefinition.GetExpansionTokens()};
                while (!macroScanner.CursorAtEnd()) {
                    // NOTE we assume that all tokens are expanded into the beginning of the macro use token.
                    PPToken token              = macroScanner.ConsumeToken();
                    token.spelledFile          = macroUseTok.spelledFile;
                    token.spelledRange         = TextRange{macroUseTok.spelledRange.start};
                    token.isFirstTokenOfLine   = false;
                    token.hasLeadingWhitespace = false;

                    // FIXME: properly handle comments
                    if (token.klass == TokenKlass::Hash) {
                        if (macroScanner.TryTestToken(TokenKlass::Identifier, 1)) {
                            // #identifier, aka. stringification
                            // However, as GLSL doesn't have string literal, we just discard two tokens.
                            // FIXME: report error, stringification is not supported.
                            macroScanner.ConsumeToken();
                            continue;
                        }

                        // Fallthrough to be handled as regular token.
                    }
                    else if (macroScanner.TryTestToken(TokenKlass::HashHash)) {
                        // token##token, aka. token pasting
                        bool pastingFailure = false;
                        std::string pastedText;
                        auto pasteTokenText = [&](const PPToken& tok) {
                            if (tok.klass == TokenKlass::Identifier) {
                                // Try substitute the parameter names with the unexpanded argument.
                                for (size_t i = 0; i < paramTokens.size(); ++i) {
                                    if (tok.text == paramTokens[i].text) {
                                        if (originalArgs[i].size() == 1) {
                                            pastedText += originalArgs[i][0].text.StrView();
                                        }
                                        else if (!originalArgs.empty()) {
                                            pastingFailure = true;
                                        }
                                        return;
                                    }
                                }
                            }

                            pastedText += tok.text.StrView();
                        };

                        // Paste all token text into a single string buffer.
                        pasteTokenText(token);
                        while (macroScanner.TryConsumeToken(TokenKlass::HashHash)) {
                            if (!macroScanner.CursorAtEnd()) {
                                pasteTokenText(macroScanner.ConsumeToken());
                            }
                            else {
                                pastingFailure = true;
                            }
                        }

                        // Try to tokenize the pasted text.
                        auto [klass, tokText, remText] = TokenizeOnce(pastedText);
                        if (!pastingFailure && remText.Empty()) {
                            token.klass = klass;
                            token.text  = atomTable.GetAtom(tokText);
                            Feed(token);
                        }
                        else {
                            // FIXME: report error, bad token pasting
                        }

                        continue;
                    }
                    else if (token.klass == TokenKlass::Identifier) {
                        // Try substitute the parameter names with the expanded argument.
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

                        if (substituted) {
                            continue;
                        }

                        // Fallthrough to be handled as regular token.
                    }

                    Feed(token);
                }
            }

            auto ExpandObjectLikeMacro(const PPToken& macroUseTok, MacroDefinition& macroDefinition) -> void
            {
                // Disable this macro to avoid recursive expansion during rescan.
                macroDefinition.Disable();
                FeedMacroExpansion(macroUseTok, macroDefinition, {}, {});
                macroDefinition.Enable();
            }

            auto ExpandFunctionLikeMacro(const PPToken& macroUseTok, MacroDefinition& macroDefinition,
                                         std::vector<PPToken> args) -> void
            {
                const auto& paramTokens = macroDefinition.GetParamTokens();

                // Collect the arguments from the token stream.
                std::vector<ArrayView<PPToken>> originalArgs;
                originalArgs.reserve(paramTokens.size());

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
                // TODO: optimize memory allocation.
                std::vector<std::vector<PPToken>> expandedArgs;
                for (auto argView : originalArgs) {
                    // FIXME: Handle error in argument expansion.
                    if constexpr (ArgExpansionMode) {
                        using BaseCallback = typename Callback::BaseCallbackType;
                        MacroExpansionProcessorImpl<MacroArgumentExpansionCallback<BaseCallback>, true> argProcessor{
                            atomTable, macroTable,
                            MacroArgumentExpansionCallback<BaseCallback>{callback.GetBaseCallback(),
                                                                         expandedArgs.emplace_back()}};

                        for (const PPToken& token : argView) {
                            argProcessor.Feed(token);
                        }
                    }
                    else {
                        using BaseCallback = Callback;
                        MacroExpansionProcessorImpl<MacroArgumentExpansionCallback<BaseCallback>, true> argProcessor{
                            atomTable, macroTable,
                            MacroArgumentExpansionCallback<BaseCallback>{callback, expandedArgs.emplace_back()}};
                        for (const PPToken& token : argView) {
                            argProcessor.Feed(token);
                        }
                    }
                }

                // Disable this macro to avoid recursive expansion during rescan.
                macroDefinition.Disable();
                FeedMacroExpansion(macroUseTok, macroDefinition, originalArgs, expandedArgs);
                macroDefinition.Enable();
            }
        };
    } // namespace detail

    template <typename Callback>
    using MacroExpansionProcessor = detail::MacroExpansionProcessorImpl<Callback, false>;
} // namespace glsld