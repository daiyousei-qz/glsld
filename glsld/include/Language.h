#pragma once
#include <string>

namespace glsld
{
    enum class SymbolKlass
    {
        Type,
        Variable,
        Function,
    };

    class Symbol
    {
    public:
        virtual auto GetKlass() -> SymbolKlass = 0;

    private:
        std::string name;
        bool is_builtin;
    };

    class TypeSymbol : public Symbol
    {
    public:
        auto GetKlass() -> SymbolKlass override
        {
            return SymbolKlass::Type;
        }

    private:
    };

    class VariableSymbol : public Symbol
    {
    public:
        auto GetKlass() -> SymbolKlass override
        {
            return SymbolKlass::Variable;
        }
    };

    class FunctionSymbol : public Symbol
    {
    public:
        auto GetKlass() -> SymbolKlass override
        {
            return SymbolKlass::Function;
        }
    };
} // namespace glsld