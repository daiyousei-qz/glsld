#pragma once

#include "Basic/Common.h"
#include "Server/Config.h"
#include "Support/JsonSerializer.h"

#include <BS_thread_pool.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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

        BS::thread_pool threadPool = {};

        std::shared_ptr<spdlog::logger> logger = nullptr;

        using MessageDispatcherType = std::function<void(LanguageServer& server, const nlohmann::json& rpcBlob)>;
        std::unordered_map<std::string, MessageDispatcherType> dispatcherMap;

        std::unique_ptr<LanguageService> language;
        std::unique_ptr<TransportService> transport;

    public:
        LanguageServer(const LanguageServerConfig& config);
        ~LanguageServer();

        auto Run() -> void;

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