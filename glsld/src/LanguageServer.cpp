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
        // Use binary mode for stdin/stdout. We handle "/r/n" conversion inhouse.
        _setmode(_fileno(stdout), O_BINARY);
        _setmode(_fileno(stdin), O_BINARY);
#else
        // FIXME: not sure if this is needed
        // freopen(nullptr, "rb", stdout);
        // freopen(nullptr, "wb", stdin);
#endif

        setvbuf(stdin, nullptr, _IOFBF, 64 * 1024);
    }
} // namespace glsld