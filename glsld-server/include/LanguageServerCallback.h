#pragma once
#include "Basic/StringView.h"
#include "Config.h"
#include "Protocol.h"

#include <BS_thread_pool.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace glsld
{
    // This is bridge interface between services and the language server
    // and it translate data between native data structure and json.
    class LanguageServerCallback
    {
    private:
        LanguageServerConfig config;

        BS::thread_pool threadPool = {};

        std::shared_ptr<spdlog::logger> logger = nullptr;

    public:
        LanguageServerCallback(const LanguageServerConfig& config) : config(config)
        {
            logger = spdlog::stderr_color_mt("glsld_logger");
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            switch (config.loggingLevel) {
            case LoggingLevel::Trace:
                logger->set_level(spdlog::level::trace);
                break;
            case LoggingLevel::Debug:
                logger->set_level(spdlog::level::debug);
                break;
            case LoggingLevel::Info:
                logger->set_level(spdlog::level::info);
                break;
            case LoggingLevel::Warn:
                logger->set_level(spdlog::level::warn);
                break;
            case LoggingLevel::Error:
                logger->set_level(spdlog::level::err);
                break;
            case LoggingLevel::Critical:
                logger->set_level(spdlog::level::critical);
                break;
            }
        }

        auto GetConfig() const -> const LanguageServerConfig&
        {
            return config;
        }

        // Submit a message from the client to language server and dispatch it to the right handler.
        auto HandleClientMessage(StringView messageText) -> void
        {
            auto message = ParseJson(messageText);
            if (message) {
                DoHandleClientMessage(std::move(*message));
            }
            else {
                DoHandleBadClientMessage(messageText);
            }
        }

        // Submit a response from the language service to language server and forward it to the client.
        template <typename T>
        auto HandleServerResponse(int requestId, const T& params, bool isError) -> void
        {
            DoHandleServerResponse(requestId, ToJson(params), isError);
        }

        // Submit a notification from the language service to language server and forward it to the client.
        template <typename T>
        auto HandleServerNotification(const char* method, const T& params) -> void
        {
            DoHandleNotification(method, ToJson(params));
        }

        template <typename F, typename... Args>
        auto ScheduleTask(F&& f, Args&&... args) -> void
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

    protected:
        virtual auto DoHandleBadClientMessage(StringView messageText) -> void = 0;

        virtual auto DoHandleClientMessage(JsonObject rpcBlob) -> void = 0;

        virtual auto DoHandleServerResponse(int requestId, JsonObject resultOrError, bool isError) -> void = 0;

        virtual auto DoHandleNotification(const char* method, JsonObject param) -> void = 0;
    };

} // namespace glsld