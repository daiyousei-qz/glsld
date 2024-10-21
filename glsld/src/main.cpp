#include "Basic/CommandLine.h"
#include "LanguageServer.h"

#if defined(GLSLD_OS_WIN)
#include <windows.h>
#endif

namespace glsld
{
    cl::Opt<bool> WaitDebugger(
        "wait-debugger", cl::ReallyHidden,
        cl::Desc("Wait for debugger to attach before starting the server. This option only works with debug build."));

#if defined(GLSLD_DEBUG)

    static auto HasDebuggerAttached() -> bool
    {
#if defined(GLSLD_OS_WIN)
        return IsDebuggerPresent() != 0;
#else
        return false;
#endif
    }

    static auto WaitDebuggerToAttach() -> void
    {
        using namespace std::literals;
        for (int i = 0; i < 10; ++i) {
            if (HasDebuggerAttached()) {
                break;
            }
            std::this_thread::sleep_for(1s);
        }
    }
#endif

    auto DoMain() -> void
    {
#if defined(GLSLD_DEBUG) && defined(GLSLD_OS_WIN)
        WaitDebuggerToAttach();
#endif

        glsld::LanguageServer{}.Run();
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
    glsld::cl::ParseArguments(argc, argv);
    glsld::DoMain();
    return 0;
}
