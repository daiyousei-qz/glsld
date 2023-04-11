#pragma once
#include "PPInfoCollector.h"

#include "Compiler.h"

#include <mutex>
#include <condition_variable>

namespace glsld
{
    class IntellisenseProvider
    {
    public:
        IntellisenseProvider(int version, std::string sourceString)
            : version(version), sourceString(std::move(sourceString))
        {
        }

        auto Setup()
        {
            compilerObject.Reset();
            compilerObject.AddIncludePath("e:/Project/glsld/.vscode/");
            compilerObject.Compile(sourceString, GetStandardLibraryModule(), &ppInfoCollector);

            std::unique_lock<std::mutex> lock{mu};
            available = true;
            cv.notify_all();
        }

        auto WaitAvailable() -> bool
        {
            using namespace std::literals;
            std::unique_lock<std::mutex> lock{mu};
            if (available || cv.wait_for(lock, 1s) == std::cv_status::no_timeout) {
                return available;
            }

            return false;
        }

        auto StealBuffer() -> std::string
        {
            std::unique_lock<std::mutex> lock{mu};
            if (available) {
                // After compilation finishes, the sourceString buffer is no longer needed
                return std::move(sourceString);
            }
            else {
                return sourceString;
            }
        }

        auto GetCompilerObject() const -> const CompilerObject&
        {
            GLSLD_ASSERT(available);
            return compilerObject;
        }

        auto GetPPInfoCache() const -> const PPInfoCache&
        {
            GLSLD_ASSERT(available);
            return ppInfoCollector;
        }

    private:
        int version;
        std::string sourceString;
        CompilerObject compilerObject;
        PPInfoCollector ppInfoCollector;

        std::atomic<bool> available = false;
        std::mutex mu;
        std::condition_variable cv;
    };
} // namespace glsld