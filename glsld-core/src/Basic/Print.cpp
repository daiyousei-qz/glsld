#include "Basic/Print.h"

#if defined(GLSLD_OS_WIN)
#include <Windows.h>
#endif

namespace glsld
{
    namespace detail
    {
        auto DebugPrintImpl(fmt::string_view fmt, fmt::format_args args) -> void
        {
#if defined(GLSLD_OS_WIN)
            auto message = fmt::vformat(fmt, args);
            OutputDebugStringA(message.c_str());
#else
            fmt::vprint(stderr, fmt, args);
#endif
        }
    } // namespace detail
} // namespace glsld