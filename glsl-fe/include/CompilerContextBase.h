#pragma once

namespace glsld
{
    template <typename ContextType>
    class CompilerContextBase
    {
    public:
        CompilerContextBase() = default;

        CompilerContextBase(const CompilerContextBase&)                    = delete;
        auto operator=(const CompilerContextBase&) -> CompilerContextBase& = delete;
        CompilerContextBase(CompilerContextBase&&)                         = delete;
        auto operator=(CompilerContextBase&&) -> CompilerContextBase&      = delete;

        static_assert(requires(ContextType ctx) { ctx.Finalize(); });
    };
} // namespace glsld