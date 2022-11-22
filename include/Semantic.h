#pragma once

namespace glsld
{
    enum class JumpType
    {
        Break,
        Continue,
        Discard,
    };

    enum class InvocationType
    {
        FunctionCall,
        Indexing,
    };
} // namespace glsld