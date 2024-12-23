#include "LanguageServer.h"

#include <argparse/argparse.hpp>

#if defined(GLSLD_OS_WIN)
#define NOMINMAX
#include <windows.h>
#endif

#if defined(GLSLD_OS_WIN)
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
    wchar_t pathBuffer[MAX_PATH];
    GetModuleFileNameW(nullptr, pathBuffer, MAX_PATH);

    wchar_t* lastSlash = wcsrchr(pathBuffer, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0'; // Keep the trailing backslash
    }

    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t dateTimeBuffer[64];
    swprintf(dateTimeBuffer, 64, L"minidump-%04d%02d%02d-%02d%02d%02d.dmp", time.wYear, time.wMonth, time.wDay,
             time.wHour, time.wMinute, time.wSecond);
    wcscat_s(pathBuffer, MAX_PATH, dateTimeBuffer);

    HANDLE hFile = CreateFileW(pathBuffer, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId          = GetCurrentThreadId();
        mdei.ExceptionPointers = pExceptionPointers;
        mdei.ClientPointers    = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &mdei, nullptr,
                          nullptr);
        CloseHandle(hFile);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void SetUnhandledExceptionFilter()
{
    ::SetUnhandledExceptionFilter(ExceptionFilter);
}
#endif

namespace glsld
{
    namespace
    {
        struct ProgramArgs
        {
        };
    } // namespace

    static auto ParseArguments(int argc, char* argv[]) -> ProgramArgs
    {
        using namespace argparse;

        ProgramArgs result;

        ArgumentParser program("glsld");
        program.add_argument("--stdio").help("Use stdio for communication").default_value(true);

        program.parse_args(argc, argv);
        return result;
    }

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

    static auto DoMain(ProgramArgs args) -> void
    {
#if defined(GLSLD_DEBUG) && defined(GLSLD_OS_WIN)
        WaitDebuggerToAttach();
#endif

        glsld::LanguageServer{}.Run();
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
#if defined(GLSLD_OS_WIN)
    SetUnhandledExceptionFilter();
#endif
    glsld::DoMain(glsld::ParseArguments(argc, argv));
    return 0;
}
