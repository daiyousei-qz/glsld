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

    enum class InterfaceBlockType
    {
        In,
        Out,
        Uniform,
        Buffer,
    };
} // namespace glsld