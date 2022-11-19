#pragma once
#include "SyntaxTree.h"

namespace glsld
{
    using SourceLocation = int;

    struct SourceLocationDesc
    {
        int file;
        int offset;
        int line;
        int column;
    };

    class SourceManager
    {
    public:
    private:
        std::string source;
    };

} // namespace glsld