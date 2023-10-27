#pragma once
#include "Basic/Common.h"

#include <charconv>
#include <functional>
#include <unordered_map>
#include <optional>
#include <span>
#include <algorithm>

// LLVM-style command line parsing facility

namespace glsld::cl
{
    class Option;

#pragma region Option Tags

    enum class ValueExpected
    {
        ValueOptional   = 0,
        ValueRequired   = 1,
        ValueDisallowed = 2,
    };

    constexpr inline ValueExpected ValueOptional   = ValueExpected::ValueOptional;
    constexpr inline ValueExpected ValueRequired   = ValueExpected::ValueRequired;
    constexpr inline ValueExpected ValueDisallowed = ValueExpected::ValueDisallowed;

    enum class OptionHidden
    {
        NotHidden    = 0,
        Hidden       = 1,
        ReallyHidden = 2,
    };

    constexpr inline OptionHidden NotHidden    = OptionHidden::NotHidden;
    constexpr inline OptionHidden Hidden       = OptionHidden::Hidden;
    constexpr inline OptionHidden ReallyHidden = OptionHidden::ReallyHidden;

    enum class FormattingFlags
    {
        NormalFormatting = 0,
        Positional       = 1,
    };

    constexpr inline FormattingFlags NormalFormatting = FormattingFlags::NormalFormatting;
    constexpr inline FormattingFlags Positional       = FormattingFlags::Positional;

    struct Category
    {
    };

    struct Desc
    {
        constexpr Desc(StringView desc) : desc(desc)
        {
        }

        StringView desc;
    };

    struct ValueDesc
    {
        constexpr ValueDesc(StringView desc) : desc(desc)
        {
        }

        StringView desc;
    };

    template <typename T>
    class Init
    {
    public:
    };

#pragma endregion

    using OptionValueParser = std::function<auto(std::optional<StringView> argValue)->void>;

    class Option
    {
    public:
        Option(StringView name) : name(name)
        {
        }

        auto GetName() const -> StringView
        {
            return name;
        }
        auto GetCategory() const -> StringView
        {
            return category;
        }
        auto GetDesc() const -> StringView
        {
            return desc;
        }
        auto GetValueDesc() const -> StringView
        {
            return valueDesc;
        }
        auto GetAlias() const -> StringView
        {
            return alias;
        }

        auto GetValueExpected() const -> ValueExpected
        {
            return valueExpected;
        }
        auto GetOptionHidden() const -> OptionHidden
        {
            return optionHidden;
        }
        auto GetFormattingFlags() const -> FormattingFlags
        {
            return formattingFlags;
        }

        virtual auto ParseValue(std::optional<StringView> argValue) -> void = 0;

    protected:
        auto SetValueExpected(ValueExpected valueExpected) -> void
        {
            this->valueExpected = valueExpected;
        }

        template <typename... Tags>
        auto ApplyOptionTags(Tags... tags) -> void
        {
            (ApplyOptionTag(tags), ...);
        }

    private:
        auto ApplyOptionTag(FormattingFlags formattingFlags) -> void
        {
            this->formattingFlags = formattingFlags;
        }

        auto ApplyOptionTag(OptionHidden optionHidden) -> void
        {
            this->optionHidden = optionHidden;
        }

        auto ApplyOptionTag(ValueExpected valueExpected) -> void
        {
            this->valueExpected = valueExpected;
        }

        auto ApplyOptionTag(Desc desc) -> void
        {
            this->desc = desc.desc;
        }

        auto ApplyOptionTag(ValueDesc desc) -> void
        {
            this->valueDesc = desc.desc;
        }

        template <typename T>
        auto ApplyOptionTag() -> void
        {
            static_assert(AlwaysFalse<T>, "unknown option tag");
        }

        friend class CommandLineContext;

        StringView name = "";

        StringView category  = "";
        StringView desc      = "";
        StringView valueDesc = "";
        StringView alias     = "";

        OptionHidden optionHidden       = NotHidden;
        ValueExpected valueExpected     = ValueRequired;
        FormattingFlags formattingFlags = NormalFormatting;
    };

    namespace detail
    {
        [[noreturn]] inline auto ExitOnBadConfig(StringView optionKey) -> void
        {
            fmt::print("bad config {}", optionKey);
            exit(-1);
        }
        [[noreturn]] inline auto ExitOnUnknownOption(StringView optionKey) -> void
        {
            fmt::print("unknown option {}", optionKey);
            exit(-1);
        }
        [[noreturn]] inline auto ExitOnBadValue(StringView optionKey, StringView typeName, StringView value) -> void
        {
            fmt::print("{} is invalid for option {} with type {}", value, optionKey, typeName);
            exit(-1);
        }

        class CommandLineContext
        {
        private:
            CommandLineContext() = default;

            CommandLineContext(const CommandLineContext&)                    = delete;
            auto operator=(const CommandLineContext&) -> CommandLineContext& = delete;

        public:
            static auto GetInstance() -> CommandLineContext&
            {
                static CommandLineContext ctx;
                return ctx;
            }

            auto RegisterOption(Option* option)
            {
                switch (option->GetFormattingFlags()) {
                case FormattingFlags::NormalFormatting:
                    assert(!option->GetName().Empty());
                    optionRegistry[option->GetName()] = option;
                    break;
                case FormattingFlags::Positional:
                    positionalOptionRegistry.push_back(option);
                    break;
                default:
                    GLSLD_UNREACHABLE();
                }
            }

            // "-o", "--option", "output file"
            auto Parse(int argc, char* argv[]) -> void
            {
                std::vector<StringView> args(argv, argv + argc);

                auto itArgs          = args.begin() + 1;
                auto positionalIndex = 0;

                auto fetchOptionArg = [&](const Option& option,
                                          std::optional<StringView> inlineValue) -> std::optional<StringView> {
                    switch (option.GetValueExpected()) {
                    case ValueOptional:
                        return inlineValue;
                    case ValueRequired:
                        if (inlineValue) {
                            return inlineValue;
                        }
                        else if (itArgs != args.end() && !itArgs->StartWith('-')) {
                            auto argValue = *itArgs;
                            ++itArgs;
                            return argValue;
                        }
                        else {
                            fmt::print("missing required arg value\n");
                            exit(-1);
                        }
                    case ValueDisallowed:
                        if (inlineValue) {
                            fmt::print("arg value is disallowed\n");
                            exit(-1);
                        }
                        return std::nullopt;
                    default:
                        GLSLD_UNREACHABLE();
                    }
                };

                while (itArgs != args.end()) {
                    auto arg0 = *itArgs;
                    ++itArgs;

                    if (arg0 == "-h" || arg0 == "-help") {
                        PrintHelp(args[0], false);
                    }
                    else if (arg0 == "-help-hidden") {
                        PrintHelp(args[0], true);
                    }

                    auto parsedOption = TryParseDashOptionArg(arg0);
                    if (parsedOption) {
                        // Normal formatting
                        if (auto itOption = optionRegistry.find(parsedOption->optionKey);
                            itOption != optionRegistry.end()) {

                            auto option   = itOption->second;
                            auto argValue = fetchOptionArg(*option, parsedOption->inlineValue);
                            option->ParseValue(argValue);
                        }
                        else {
                            // ExitOnUnknownOption(parsedOption->optionKey);
                        }
                    }
                    else {
                        // Positional
                        if (positionalIndex < positionalOptionRegistry.size()) {
                            auto option   = positionalOptionRegistry[positionalIndex];
                            auto argValue = arg0;

                            option->ParseValue(argValue);
                        }
                        else {
                            fmt::print("additional positional option");
                            exit(-1);
                        }
                        ++positionalIndex;
                    }
                }
            }

            auto PrintHelp(StringView appName, bool showHidden) -> void
            {
                fmt::print("{} [options...]", appName);
                for (const Option* option : positionalOptionRegistry) {
                    fmt::print(" <{}>", !option->GetValueDesc().Empty() ? option->GetValueDesc() : "?");
                }
                fmt::print("\n\n");

                fmt::print("Options:\n");
                fmt::print("  -h, -help  Print this help message.\n");
                for (auto [optionKey, option] : optionRegistry) {
                    if (option->GetOptionHidden() == ReallyHidden) {
                        // Never print help for really hidden options
                        continue;
                    }
                    if (option->GetOptionHidden() == Hidden && !showHidden) {
                        // Don't print help for hidden options if not asked
                        continue;
                    }

                    fmt::print("  -{}  {}\n", optionKey, option->GetDesc());
                }

                exit(-1);
            }

        private:
            struct ParsedOptionArg
            {
                int numLeadingDash;
                StringView optionKey;
                std::optional<StringView> inlineValue;
            };

            // Try parse an option arg like "-o", "--option", "-o=foo", "--option=foo"
            auto TryParseDashOptionArg(StringView arg) -> std::optional<ParsedOptionArg>
            {
                if (!arg.StartWith('-')) {
                    return std::nullopt;
                }

                // NOTE it is guranteed that `itOptionNameBegin` comes before `itEqualBegin`
                auto itOptionNameBegin = std::ranges::find_if(arg, [](char ch) { return ch != '-'; });
                auto itEqualBegin      = std::ranges::find_if(arg, [](char ch) { return ch == '='; });

                int numLeadingDash = std::distance(arg.begin(), itOptionNameBegin);
                auto optionKey     = StringView(itOptionNameBegin, itEqualBegin);
                auto inlineValue =
                    itEqualBegin != arg.end() ? std::optional{StringView{itEqualBegin + 1, arg.end()}} : std::nullopt;

                return ParsedOptionArg{
                    .numLeadingDash = numLeadingDash,
                    .optionKey      = optionKey,
                    .inlineValue    = inlineValue,
                };
            }

            std::unordered_map<StringView, Option*> optionRegistry;
            std::vector<Option*> positionalOptionRegistry;
        };
    } // namespace detail

    template <typename T>
    class Opt : public Option
    {
    public:
        template <typename... Tags>
        explicit Opt(const char* name, Tags... tags) : Option(name)
        {
            Initialize();
            ApplyOptionTags(tags...);
            detail::CommandLineContext::GetInstance().RegisterOption(this);
        }
        template <typename... Tags>
        explicit Opt(Tags... tags) : Option("")
        {
            Initialize();
            ApplyOptionTags(tags...);
            detail::CommandLineContext::GetInstance().RegisterOption(this);
        }

        Opt(const Opt&)                    = delete;
        auto operator=(const Opt&) -> Opt& = delete;

        auto HasValue() -> bool
        {
            return value.has_value();
        }

        auto GetValue() -> T&
        {
            return *value;
        }

        virtual auto ParseValue(std::optional<StringView> argValue) -> void override
        {
            if constexpr (std::is_same_v<T, bool>) {
                SetValueExpected(ValueOptional);
                if (argValue) {
                    if (argValue == "true") {
                        value = true;
                    }
                    else if (argValue == "false") {
                        value = false;
                    }
                    else {
                        detail::ExitOnBadValue(GetName(), "bool", *argValue);
                    }
                }
                else {
                    value = true;
                }
            }
            else if constexpr (std::is_integral_v<T>) {
                if (argValue) {
                    T tmp;
                    auto parseResult = std::from_chars(argValue->data(), argValue->data() + argValue->Size(), tmp);
                    if (parseResult.ec == std::errc() && parseResult.ptr == argValue->data() + argValue->Size()) {
                        value = tmp;
                    }
                    else {
                        detail::ExitOnBadValue(GetName(), "int", *argValue);
                    }
                }
            }
            else if constexpr (std::is_floating_point_v<T>) {
                if (argValue) {
                    T tmp;
                    auto parseResult = std::from_chars(argValue->data(), argValue->data() + argValue->Size(), tmp);
                    if (parseResult.ec == std::errc() && parseResult.ptr == argValue->data() + argValue->Size()) {
                        value = tmp;
                    }
                    else {
                        detail::ExitOnBadValue(GetName(), "float", *argValue);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                if (argValue) {
                    value = argValue->Str();
                }
            }
            else {
                static_assert(AlwaysFalse<T>);
            }
        }

    private:
        auto Initialize() -> void
        {
            if constexpr (std::is_same_v<T, bool>) {
                SetValueExpected(ValueOptional);
            }
            else {
                SetValueExpected(ValueRequired);
            }
        }

        std::optional<T> value;
    };

    inline auto ParseArguments(int argc, char* argv[]) -> void
    {
        detail::CommandLineContext::GetInstance().Parse(argc, argv);
    }
} // namespace glsld::cl