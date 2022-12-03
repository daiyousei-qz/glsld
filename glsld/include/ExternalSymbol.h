#pragma once
#include "Typing.h"
#include <string>

namespace glsld
{
    enum class ExternalSymbolType
    {
        Variable,
        Function,
        Type,
    };

    struct ExternalFunctionSymbol
    {
        std::string name;
        std::string documentation;

        const TypeDesc* returnType;
        std::vector<const TypeDesc*> paramTypes;
    };
} // namespace glsld