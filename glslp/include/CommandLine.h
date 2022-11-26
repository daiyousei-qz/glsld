#pragma once
#include <string_view>
#include <functional>
#include <unordered_map>
#include <optional>
#include <span>

#include <fmt/format.h>

// LLVM-style command line parsing facility

namespace cl
{
    class Option;

    [[noreturn]] inline auto Unreachable() -> void
    {
        std::abort();
    }

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
        NormalFormatting,
        Positional,
    };

    constexpr inline FormattingFlags NormalFormatting = FormattingFlags::NormalFormatting;
    constexpr inline FormattingFlags Positional       = FormattingFlags::Positional;

    struct Category
    {
    };

    struct Desc
    {
        consteval Desc(std::string_view desc) : desc(desc)
        {
        }

        std::string_view desc;
    };

    struct ValueDesc
    {
        consteval ValueDesc(std::string_view desc) : desc(desc)
        {
        }

        std::string_view desc;
    };

    class AliasOpt
    {
    public:
        consteval AliasOpt(Option& alias) : alias(&alias)
        {
        }

        Option* alias;
    };

    template <typename T>
    class Init
    {
    public:
    };

    using OptionValueHandler = std::function<auto(std::optional<std::string_view> argValue)->void>;

    class Option
    {
    public:
        auto GetName() const -> std::string_view
        {
            return name;
        }
        auto GetCategory() const -> std::string_view
        {
            return category;
        }
        auto GetDesc() const -> std::string_view
        {
            return desc;
        }
        auto GetValueDesc() const -> std::string_view
        {
            return valueDesc;
        }
        auto GetAlias() const -> std::string_view
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

        auto GetHandler() const -> const OptionValueHandler&
        {
            return handler;
        }

    protected:
        auto SetOptionName(std::string_view name) -> void
        {
            this->name = name;
        }

        auto SetValueExpected(ValueExpected valueExpected) -> void
        {
            this->valueExpected = valueExpected;
        }

        auto SetOptionHidden(OptionHidden optionHidden) -> void
        {
            this->optionHidden = optionHidden;
        }

        auto SetFormattingFlags(FormattingFlags formattingFlags) -> void
        {
            this->formattingFlags = formattingFlags;
        }

        auto SetOptionValueHandler(OptionValueHandler handler) -> void
        {
            this->handler = handler;
        }

        class AttributeParser
        {
        public:
            AttributeParser(Option& option) : option(option)
            {
            }

            template <typename... Modifiers>
            auto Apply(Modifiers... mods) -> void
            {
                (Accept(mods), ...);
            }

        private:
            auto Accept(FormattingFlags formattingFlags) -> void
            {
                option.SetFormattingFlags(formattingFlags);
            }

            auto Accept(OptionHidden optionHidden) -> void
            {
                option.SetOptionHidden(optionHidden);
            }

            auto Accept(ValueExpected valueExpected) -> void
            {
                option.SetValueExpected(valueExpected);
            }

            auto Accept(Desc desc) -> void
            {
                option.desc = desc.desc;
            }

            auto Accept(ValueDesc desc) -> void
            {
                option.valueDesc = desc.desc;
            }

            auto Accept(AliasOpt alias) -> void
            {
                option.alias = alias.alias->GetName();
            }

            Option& option;
        };

        template <typename... Attributes>
        auto ApplyOptionAttributes(Attributes... attrs) -> void
        {
            AttributeParser parser(*this);
            parser.Apply(attrs...);
        }

    private:
        std::string_view name      = "";
        std::string_view category  = "";
        std::string_view desc      = "";
        std::string_view valueDesc = "";
        std::string_view alias     = "";

        OptionHidden optionHidden       = NotHidden;
        ValueExpected valueExpected     = ValueRequired;
        FormattingFlags formattingFlags = NormalFormatting;

        OptionValueHandler handler;
    };

    namespace detail
    {
        [[noreturn]] inline auto ExitOnUnknownOption(std::string_view optionKey) -> void
        {
            fmt::print("unknown option {}", optionKey);
            exit(-1);
        }
        [[noreturn]] inline auto ExitOnBadValue(std::string_view optionKey, std::string_view typeName,
                                                std::string_view value) -> void
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

            auto Register(Option* option)
            {
                switch (option->GetFormattingFlags()) {
                case FormattingFlags::NormalFormatting:
                    optionRegistry[option->GetName()] = option;
                    break;
                case FormattingFlags::Positional:
                    positionalOptionRegistry.push_back(option);
                    break;
                default:
                    Unreachable();
                }
            }

            auto Parse(int argc, char* argv[]) -> void
            {
                std::vector<std::string_view> args(argv, argv + argc);

                auto itArgs          = args.begin() + 1;
                auto positionalIndex = 0;

                auto fetchOptionArg =
                    [&](const Option& option,
                        std::optional<std::string_view> inlineValue) -> std::optional<std::string_view> {
                    switch (option.GetValueExpected()) {
                    case ValueOptional:
                        return inlineValue;
                    case ValueRequired:
                        if (inlineValue) {
                            return inlineValue;
                        }
                        else if (itArgs != args.end() && !itArgs->starts_with('-')) {
                            auto argValue = *itArgs;
                            ++itArgs;
                            return argValue;
                        }
                        else {
                            printf("missing required arg value\n");
                            exit(-1);
                        }
                    case ValueDisallowed:
                        if (inlineValue) {
                            printf("arg value is disallowed\n");
                            exit(-1);
                        }
                        return std::nullopt;
                    default:
                        Unreachable();
                    }
                };

                while (itArgs != args.end()) {
                    auto arg0 = *itArgs;
                    ++itArgs;

                    if (arg0 == "-h" || arg0 == "-help") {
                        PrintHelp(args[0]);
                    }

                    auto parsedOption = ParseOptionArg(arg0);
                    if (parsedOption) {
                        if (auto itOption = optionRegistry.find(parsedOption->optionKey);
                            itOption != optionRegistry.end()) {

                            // dispatch
                            auto option   = itOption->second;
                            auto argValue = fetchOptionArg(*option, parsedOption->inlineValue);

                            option->GetHandler()(argValue);
                        }
                        else {
                            ExitOnUnknownOption(parsedOption->optionKey);
                        }
                    }
                    else {
                        if (positionalIndex < positionalOptionRegistry.size()) {
                            auto option   = positionalOptionRegistry[positionalIndex];
                            auto argValue = arg0;

                            option->GetHandler()(argValue);
                        }
                        else {
                            // positional
                            fmt::print("additional positional option");
                            exit(-1);
                        }
                        ++positionalIndex;
                    }
                }
            }

            auto PrintHelp(std::string_view appName) -> void
            {
                fmt::print("{} [options] ...\n\n", appName);

                fmt::print("Options:\n");
                for (auto [optionKey, option] : optionRegistry) {
                    fmt::print("-{}\n", optionKey);
                }

                exit(-1);
            }

        private:
            struct ParsedOptionArg
            {
                int numLeadingDash;
                std::string_view optionKey;
                std::optional<std::string_view> inlineValue;
            };

            auto ParseOptionArg(std::string_view arg) -> std::optional<ParsedOptionArg>
            {
                if (!arg.starts_with('-')) {
                    return std::nullopt;
                }

                // NOTE it is guranteed that `itOptionNameBegin` comes before `itEqualBegin`
                auto itOptionNameBegin = std::ranges::find_if(arg, [](char ch) { return ch != '-'; });
                auto itEqualBegin      = std::ranges::find_if(arg, [](char ch) { return ch == '='; });

                int numLeadingDash = std::distance(arg.begin(), itOptionNameBegin);
                auto optionKey     = std::string_view(itOptionNameBegin, itEqualBegin);
                auto inlineValue   = itEqualBegin != arg.end()
                                         ? std::optional{std::string_view{itEqualBegin + 1, arg.end()}}
                                         : std::nullopt;

                return ParsedOptionArg{
                    .numLeadingDash = numLeadingDash,
                    .optionKey      = optionKey,
                    .inlineValue    = inlineValue,
                };
            }

            std::unordered_map<std::string_view, Option*> optionRegistry;
            std::vector<Option*> positionalOptionRegistry;
        };
    } // namespace detail

    template <typename T>
    class Opt : public Option
    {
    public:
        template <typename... Tags>
        explicit Opt(const char* name, Tags... tags)
        {
            SetOptionName(name);
            Initialize();
            ApplyOptionAttributes(tags...);
            detail::CommandLineContext::GetInstance().Register(this);
        }
        template <typename... Tags>
        explicit Opt(Tags... tags)
        {
            Initialize();
            ApplyOptionAttributes(tags...);
            detail::CommandLineContext::GetInstance().Register(this);
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

    private:
        auto Initialize()
        {
            if constexpr (std::is_same_v<T, bool>) {
                SetValueExpected(ValueOptional);
                SetOptionValueHandler([this](std::optional<std::string_view> argValue) -> void {
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
                });
            }
            else if constexpr (std::is_integral_v<T>) {
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                SetOptionValueHandler([this](std::optional<std::string_view> argValue) -> void {
                    if (argValue) {
                        value = *argValue;
                    }
                });
            }
        }

        std::optional<T> value;
    };

    inline auto ParseArguments(int argc, char* argv[]) -> void
    {
        detail::CommandLineContext::GetInstance().Parse(argc, argv);
    }
} // namespace cl