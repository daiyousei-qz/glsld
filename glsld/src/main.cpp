#include "Support/StringView.h"
#include "Support/File.h"
#include "Server/LanguageServer.h"

#include <argparse/argparse.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>

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
            // Path to the configuration file
            std::string configFile;
            // Path to a file containing LSP messages to replay a sequence of client messages
            std::string replayFile;
            // Path to the directory to dump replay files
            std::string replayDumpDir;
        };
    } // namespace

    static auto ParseArguments(int argc, char* argv[]) -> ProgramArgs
    {
        using namespace argparse;

        ProgramArgs result;

        ArgumentParser program("glsld");
        program.add_argument("--stdio").help("Use stdio for communication").default_value(true);
        program.add_argument("--configFile")
            .help("Path to the configuration file")
            .default_value(std::string{})
            .store_into(result.configFile);
        program.add_argument("--replayFile")
            .help("Path to a file containing LSP messages to replay a sequence of client messages")
            .default_value(std::string{})
            .store_into(result.replayFile);
        program.add_argument("--replayDumpDir")
            .help("Directory to dump the replay file (for reproducing client messages)")
            .default_value(std::string{})
            .store_into(result.replayDumpDir);

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

    static auto LoadConfig(const std::string& configFilePath) -> LanguageServerConfig
    {
        if (configFilePath.empty()) {
            return LanguageServerConfig{};
        }

        std::ifstream configFileStream(configFilePath);
        if (!configFileStream.is_open()) {
            fmt::print(stderr, "Failed to open config file: {}\n", configFilePath);
            std::exit(1);
        }
        std::stringstream buffer;
        buffer << configFileStream.rdbuf();

        std::string configContent = buffer.str();
        auto config               = ParseLanguageServerConfig(configContent);

        if (!config) {
            fmt::print(stderr, "Failed to parse config file: {}\n", configFilePath);
            std::exit(1);
        }

        return *config;
    }

    static auto CreateReplayDumpFile(StringView dumpDir) -> UniqueFile
    {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tm;
#if defined(GLSLD_OS_WIN)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        auto path = std::filesystem::path{dumpDir.StdStrView()} /
                    fmt::format("glsld-replay-{:02d}_{:02d}-{:02d}_{:02d}_{:02d}_{:02d}.{:03d}.txt", tm.tm_mon + 1,
                                tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_year + 1900, ms.count());

        return UniqueFile::Open(path.string().c_str(), "wb").value_or(UniqueFile{});
    }

    static auto DoMain(ProgramArgs args) -> int
    {
        auto config = LoadConfig(args.configFile);
        glsld::LanguageServer server{config};

        if (!args.replayFile.empty()) {
            auto replayCommands = UniqueFile::ReadAllText(args.replayFile.c_str());
            if (!replayCommands) {
                fmt::print(stderr, "Failed to read replay file: {}\n", args.replayFile);
                return 1;
            }

            server.Replay(*replayCommands);
            return 1;
        }
        else {
            if (!args.replayDumpDir.empty()) {
                auto replayDumpFile = CreateReplayDumpFile(args.replayDumpDir);
                if (!replayDumpFile) {
                    fmt::print(stderr, "Failed to initialize replay file at: {}\n", args.replayDumpDir);
                    return 1;
                }

                server.InitializeReplayDumpFile(std::move(replayDumpFile));
            }
            server.Run();
        }
        return 0;
    }
} // namespace glsld

auto main(int argc, char* argv[]) -> int
{
#if defined(GLSLD_DEBUG) && defined(GLSLD_OS_WIN)
    glsld::WaitDebuggerToAttach();
#endif

#if defined(GLSLD_OS_WIN)
    SetUnhandledExceptionFilter();
#endif

    return glsld::DoMain(glsld::ParseArguments(argc, argv));
}
