#include <cstdio>

#if defined(GLSLD_OS_WIN)
#include <io.h>
#include <fcntl.h>
#endif

namespace glsld
{
    auto InitializeStdIO() -> void
    {
#if defined(GLSLD_OS_WIN)
        _setmode(_fileno(stdout), O_BINARY);
        _setmode(_fileno(stdin), O_BINARY);
#endif
    }
} // namespace glsld