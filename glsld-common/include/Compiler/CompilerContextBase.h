#pragma once
#include <type_traits>

namespace glsld
{
    template <typename Derived>
    class CompilerContextBase
    {
    private:
        const Derived* preambleContext = nullptr;

    public:
        CompilerContextBase(const Derived* preambleContext) : preambleContext(preambleContext)
        {
            static_assert(std::is_base_of_v<CompilerContextBase, Derived>);
        }

        CompilerContextBase(const CompilerContextBase&)                    = delete;
        CompilerContextBase(CompilerContextBase&&)                         = delete;
        auto operator=(const CompilerContextBase&) -> CompilerContextBase& = delete;
        auto operator=(CompilerContextBase&&) -> CompilerContextBase&      = delete;

        auto GetPreambleContext() const noexcept -> const Derived*
        {
            return preambleContext;
        }
    };
} // namespace glsld