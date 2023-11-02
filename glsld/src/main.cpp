#include "Basic/CommandLine.h"
#include "LanguageServer.h"

#if defined(GLSLD_OS_WIN)
// #include <debugapi.h>
#endif

#include <vector>
#include <nlohmann/json.hpp>

namespace glsld
{
    cl::Opt<bool> WaitDebugger(
        "wait-debugger", cl::ReallyHidden,
        cl::Desc("Wait for debugger to attach before starting the server. This option only works with debug build."));

#if defined(GLSLD_DEBUG)

    auto HasDebuggerAttached() -> bool
    {
#if defined(GLSLD_OS_WIN)
        return false;
#elif defined(GLSLD_OS_LINUX)
        return false;
#else
        return false;
#endif
    }

    auto WaitDebuggerToAttach() -> void
    {
        using namespace std::literals;
        std::this_thread::sleep_for(10s);
    }
#endif

    auto DoMain() -> void
    {
#if defined(GLSLD_DEBUG)
        // WaitDebuggerToAttach();
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
