#pragma once

#include "Basic/Common.h"
#include "Server/Config.h"
#include "Support/JsonSerializer.h"

#include <BS_thread_pool.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

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
        auto Initialize() -> void;

        auto DoHandleServerResponse(int requestId, nlohmann::json result, bool isError) -> void;
        auto DoHandleNotification(const char* method, nlohmann::json params) -> void;

        template <typename ParamType>
        using RequestHandlerType = void (LanguageService::*)(int requestId, ParamType params);

        template <typename ParamType>
        auto AddRequestHandler(StringView methodName, RequestHandlerType<ParamType> handler) -> void
        {
            auto [it, inserted] = dispatcherMap.insert_or_assign(
                methodName.Str(), [handler](LanguageServer& server, const nlohmann::json& rpcBlob) {
                    const auto& jreqid = rpcBlob["id"];
                    if (!jreqid.is_number_integer()) {
                        server.LogError("JSON-RPC request ID must be a valid integer.");
                        return;
                    }
                    int requestId = jreqid;

                    ParamType params;
                    if (JsonSerializer<ParamType>::Deserialize(params, rpcBlob["params"])) {
                        std::invoke(handler, server.language.get(), requestId, std::move(params));
                    }
                    else {
                        server.LogError("Failed to deserialize JSON-RPC request parameters.");
                    }
                });
            GLSLD_ASSERT(inserted);
        }

        template <typename ParamType>
        using NotificationHandlerType = void (LanguageService::*)(ParamType params);

        template <typename ParamType>
        auto AddNotificationHandler(StringView methodName, NotificationHandlerType<ParamType> handler) -> void
        {
            auto [it, inserted] = dispatcherMap.insert_or_assign(
                methodName.Str(), [handler](LanguageServer& server, const nlohmann::json& rpcBlob) {
                    ParamType params;
                    if (JsonSerializer<ParamType>::Deserialize(params, rpcBlob["params"])) {
                        std::invoke(handler, server.language.get(), std::move(params));
                    }
                    else {
                        server.LogError("Failed to deserialize JSON-RPC notification parameters.");
                    }
                });
            GLSLD_ASSERT(inserted);
        }
    };
} // namespace glsld