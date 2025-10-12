#pragma once

#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Server/Config.h"
#include "Support/JsonSerializer.h"

#include <BS_thread_pool.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/logger.h>

#include <cstdio>
#include <memory>
#include <unordered_map>
#include <functional>

namespace glsld
{
    class LanguageService;
    class TransportService;

    // handles jsonrpc
    class LanguageServer
    {
    private:
        LanguageServerConfig config;

        BS::light_thread_pool threadPool = {
            0, [](std::size_t index) { BS::this_thread::set_os_thread_name("glsld_worker_" + std::to_string(index)); }};

        std::shared_ptr<spdlog::logger> logger = nullptr;

        std::unique_ptr<FILE, decltype(&fclose)> replayFile{nullptr, fclose};

        using MessageDispatcherType = std::function<void(LanguageServer& server, const nlohmann::json& rpcBlob)>;
        std::unordered_map<std::string, MessageDispatcherType> dispatcherMap;

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TransportService> transport;

    public:
        LanguageServer(const LanguageServerConfig& config);
        ~LanguageServer();

        auto InitializeReplayDumpFile(StringView dirPath) -> bool;

        auto Run() -> void;

        auto Replay(std::string replayCommands) -> void;

        auto GetConfig() const -> const LanguageServerConfig&
        {
            return config;
        }

        auto GetLanguageService() -> LanguageService&
        {
            return *language;
        }

        auto GetTransportService() -> TransportService&
        {
            return *transport;
        }

        // Dispatch a message from the client to the language server.
        auto HandleClientMessage(StringView messagePayload) -> void;

        // Dispatch a response from the language server to the client.
        template <typename T>
        auto HandleServerResponse(int requestId, const T& result, bool isError) -> void
        {
            DoHandleServerResponse(requestId, JsonSerializer<T>::Serialize(result), isError);
        }

        // Dispatch a notification from the language server to the client.
        template <typename T>
        auto HandleServerNotification(const char* method, const T& params) -> void
        {
            DoHandleNotification(method, JsonSerializer<T>::Serialize(params));
        }

        template <typename F, typename... Args>
        auto ScheduleBackgroundTask(F&& f, Args&&... args) -> void
        {
            threadPool.detach_task(std::forward<F>(f), std::forward<Args>(args)...);
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
            if (replayFile) {
                constexpr StringView separator = ",\n\n";
                fwrite(messagePayload.data(), 1, messagePayload.Size(), replayFile.get());
                fwrite(separator.data(), 1, separator.Size(), replayFile.get());
                fflush(replayFile.get());
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
        auto DoHandleServerResponse(int requestId, nlohmann::json result, bool isError) -> void;
        auto DoHandleNotification(const char* method, nlohmann::json params) -> void;

        auto AddClientMessageHandler(StringView methodName,
                                     std::function<auto(LanguageServer&, const nlohmann::json&)->void> handler) -> void;
    };
} // namespace glsld