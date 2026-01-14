#pragma once

#include "Support/File.h"
#include "Support/JsonSerializer.h"
#include "Support/StringMap.h"
#include "Support/StringView.h"
#include "Server/Config.h"
#include "Server/TextTransport.h"

#include <nlohmann/json.hpp>
#include <spdlog/logger.h>

#include <cstdio>
#include <memory>
#include <functional>
#include <stop_token>

namespace glsld
{
    class LanguageService;

    // handles jsonrpc
    class LanguageServer
    {
    private:
        LanguageServerConfig config;

        std::shared_ptr<spdlog::logger> logger = nullptr;

        // Replay file is formatted like a JSON array, but excluding the enclosing brackets.
        // This is to ensure that crashing server doesn't produce invalid replay file. A final json should be composed
        // while replay file is loaded.
        UniqueFile replayDumpFile;

        using ClientMessageHandlerType =
            std::function<auto(LanguageServer& server, const nlohmann::json& rpcBlob)->void>;
        UnorderedStringMap<ClientMessageHandlerType> handlerDispatchMap;

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TextTransport> transport;

        // This controls the server listening loop.
        std::stop_source serverStopSource;

        // While server is always listening in the main thread, outgoing messages may be sent from different threads.
        // This mutex protects the output to be thread-safe.
        std::mutex serverOutputMutex;

    public:
        LanguageServer(const LanguageServerConfig& config);
        ~LanguageServer();

        auto InitializeReplayDumpFile(UniqueFile dumpFile) -> void;

        auto Run() -> void;

        auto Replay(std::string replayCommands) -> void;

        auto Shutdown() -> void
        {
            serverStopSource.request_stop();
        }

        auto GetConfig() const -> const LanguageServerConfig&
        {
            return config;
        }

        // Dispatch a response from the language server to the client.
        // This function is thread-safe.
        template <typename T>
        auto SendServerResponse(int requestId, const T& result, bool isError) -> void
        {
            SendTypeErasedServerResponse(requestId, JsonSerializer<T>::Serialize(result), isError);
        }

        // Dispatch a notification from the language server to the client.
        // This function is thread-safe.
        template <typename T>
        auto SendServerNotification(const char* method, const T& params) -> void
        {
            SendTypeErasedServerNotification(method, JsonSerializer<T>::Serialize(params));
        }

        auto ShouldLog(LoggingLevel requiredLevel) const -> bool
        {
            switch (config.loggingLevel.value) {
            case LoggingLevel::Debug:
                return true;
            case LoggingLevel::Info:
                return requiredLevel != LoggingLevel::Debug;
            case LoggingLevel::Warn:
                return requiredLevel == LoggingLevel::Warn || requiredLevel == LoggingLevel::Error;
            case LoggingLevel::Error:
                return requiredLevel == LoggingLevel::Error;
            default:
                return false;
            }
        }

        auto LogClientMessage(StringView messagePayload) -> void
        {
            if (replayDumpFile) {
                replayDumpFile.Write(messagePayload);
                replayDumpFile.Write(",\n\n");
                replayDumpFile.Flush();
            }
        }

        template <typename... Args>
        auto LogDebug(fmt::format_string<Args...> format, Args&&... args) -> void
        {
            if (logger) {
                logger->debug(format, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        auto LogInfo(fmt::format_string<Args...> format, Args&&... args) -> void
        {
            if (logger) {
                logger->info(format, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        auto LogWarn(fmt::format_string<Args...> format, Args&&... args) -> void
        {
            if (logger) {
                logger->warn(format, std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        auto LogError(fmt::format_string<Args...> format, Args&&... args) -> void
        {
            if (logger) {
                logger->error(format, std::forward<Args>(args)...);
            }
        }

    private:
        // Synchronously pull a client message from the input transport and forward it to the server.
        // This should be called in the main thread within our listening loop.
        // Returns true if a message is successfully pulled.
        auto PullMessage() -> bool;

        // Synchronously push a server message to the output transport.
        // Because this may be called from different worker threads, this function locks the output mutex.
        // Returns true if a message is successfully pushed.
        auto PushMessage(StringView payload) -> bool;

        auto HandleClientMessage(StringView messagePayload) -> void;
        auto SendTypeErasedServerResponse(int requestId, nlohmann::json result, bool isError) -> void;
        auto SendTypeErasedServerNotification(const char* method, nlohmann::json params) -> void;

        auto InitializeClientMessageHandlers() -> void;
    };
} // namespace glsld