#include "Server/LanguageServer.h"
#include "Server/LanguageService.h"
#include "Server/Protocol.h"
#include "Server/TransportService.h"

#include <cctype>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <cstdio>

#if defined(GLSLD_OS_WIN)
#include <io.h>
#include <fcntl.h>
#endif

namespace glsld
{
    static auto CreateDefaultLanguageService(LanguageServer& server) -> std::unique_ptr<LanguageService>
    {
        return std::make_unique<LanguageService>(server);
    }

    static auto CreateStdioTransportService(LanguageServer& server) -> std::unique_ptr<TransportService>
    {
#if defined(GLSLD_OS_WIN)
        // Use binary mode for stdin/stdout. We handle "/r/n" conversion inhouse.
        _setmode(_fileno(stdout), O_BINARY);
        _setmode(_fileno(stdin), O_BINARY);
#else
        // FIXME: not sure if this is needed
        // freopen(nullptr, "rb", stdout);
        // freopen(nullptr, "wb", stdin);
#endif

        setvbuf(stdin, nullptr, _IOFBF, 64 * 1024);
        return std::make_unique<TransportService>(server, stdin, stdout);
    }

    static auto CreateLogger(LoggingLevel level) -> std::shared_ptr<spdlog::logger>
    {
        auto logger = spdlog::stderr_color_mt("glsld_logger");
        logger->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
        switch (level) {
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
        }

        return logger;
    }

    template <typename ParamType>
    using RequestHandlerType = void (LanguageService::*)(int requestId, ParamType params);

    template <typename ParamType>
    static auto CreateRequestHandler(RequestHandlerType<ParamType> handler, bool allowNoParams = false)
        -> std::function<auto(LanguageServer&, const nlohmann::json&)->void>
    {
        return [handler, allowNoParams](LanguageServer& server, const nlohmann::json& rpcBlob) {
            int requestId;
            if (auto itReqId = rpcBlob.find("id"); itReqId != rpcBlob.end() && itReqId->is_number_integer()) {
                requestId = *itReqId;
            }
            else {
                server.LogError("JSON-RPC request ID must be a valid integer.");
                return;
            }

            ParamType params;
            if (auto itParam = rpcBlob.find("params"); itParam != rpcBlob.end()) {
                if (JsonSerializer<ParamType>::Deserialize(params, rpcBlob["params"])) {
                    std::invoke(handler, &server.GetLanguageService(), requestId, std::move(params));
                }
                else {
                    server.LogError("Failed to deserialize JSON-RPC request parameters.");
                }
            }
            else if (allowNoParams) {
                std::invoke(handler, &server.GetLanguageService(), requestId, params);
            }
        };
    }

    template <typename ParamType>
    using NotificationHandlerType = void (LanguageService::*)(ParamType params);

    template <typename ParamType>
    static auto CreateNotificationHandler(NotificationHandlerType<ParamType> handler, bool allowNoParams = false)
        -> std::function<auto(LanguageServer&, const nlohmann::json&)->void>
    {
        return [handler, allowNoParams](LanguageServer& server, const nlohmann::json& rpcBlob) {
            ParamType params;
            if (auto itParam = rpcBlob.find("params"); itParam != rpcBlob.end()) {
                if (JsonSerializer<ParamType>::Deserialize(params, rpcBlob["params"])) {
                    std::invoke(handler, &server.GetLanguageService(), std::move(params));
                }
                else {
                    server.LogError("Failed to deserialize JSON-RPC notification parameters.");
                }
            }
            else if (allowNoParams) {
                std::invoke(handler, &server.GetLanguageService(), params);
            }
        };
    }

    LanguageServer::LanguageServer(const LanguageServerConfig& config) : config(config)
    {
        logger = CreateLogger(config.loggingLevel);

        language  = CreateDefaultLanguageService(*this);
        transport = CreateStdioTransportService(*this);

        AddClientMessageHandler(lsp::LSPMethod_Initialize, CreateRequestHandler(&LanguageService::Initialize));
        AddClientMessageHandler(lsp::LSPMethod_Initialized, [](LanguageServer&, const nlohmann::json&) {});
        AddClientMessageHandler(lsp::LSPMethod_SetTrace, [](LanguageServer&, const nlohmann::json&) {});
        AddClientMessageHandler(lsp::LSPMethod_Shutdown, CreateRequestHandler(&LanguageService::Shutdown, true));
        AddClientMessageHandler(lsp::LSPMethod_Exit, CreateNotificationHandler(&LanguageService::Exit, true));

        AddClientMessageHandler(lsp::LSPMethod_DocumentSymbol, CreateRequestHandler(&LanguageService::DocumentSymbol));
        AddClientMessageHandler(lsp::LSPMethod_SemanticTokensFull,
                                CreateRequestHandler(&LanguageService::SemanticTokensFull));
        AddClientMessageHandler(lsp::LSPMethod_Completion, CreateRequestHandler(&LanguageService::Completion));
        AddClientMessageHandler(lsp::LSPMethod_SignatureHelp, CreateRequestHandler(&LanguageService::SignatureHelp));
        AddClientMessageHandler(lsp::LSPMethod_Hover, CreateRequestHandler(&LanguageService::Hover));
        AddClientMessageHandler(lsp::LSPMethod_References, CreateRequestHandler(&LanguageService::References));
        AddClientMessageHandler(lsp::LSPMethod_Definition, CreateRequestHandler(&LanguageService::Definition));
        AddClientMessageHandler(lsp::LSPMethod_InlayHint, CreateRequestHandler(&LanguageService::InlayHint));

        AddClientMessageHandler(lsp::LSPMethod_DidOpenTextDocument,
                                CreateNotificationHandler(&LanguageService::DidOpenTextDocument));
        AddClientMessageHandler(lsp::LSPMethod_DidChangeTextDocument,
                                CreateNotificationHandler(&LanguageService::DidChangeTextDocument));
        AddClientMessageHandler(lsp::LSPMethod_DidCloseTextDocument,
                                CreateNotificationHandler(&LanguageService::DidCloseTextDocument));
    }
    LanguageServer::~LanguageServer()
    {
    }

    auto LanguageServer::InitializeReplayDumpFile(UniqueFile dumpFile) -> void
    {
        GLSLD_ASSERT(!replayDumpFile);
        replayDumpFile = std::move(dumpFile);
    }

    auto LanguageServer::Run() -> void
    {
        running = true;
        while (running) {
            auto message = transport->PullMessage();
            if (!message.has_value()) {
                break;
            }

            DoHandleClientMessage(*message);
        }

        threadPool.wait();
        running = false;
    }

    auto LanguageServer::Replay(std::string replayCommands) -> void
    {
        // FIXME: The current version of nlohmann::json we are using doesn't like trailing comma (yet)
        //        We should let the library tolarate the comma after upgrading.
        while (!replayCommands.empty() && (isspace(replayCommands.back()) || replayCommands.back() == ',')) {
            replayCommands.pop_back();
        }
        replayCommands = "[" + replayCommands + "]";

        auto j = nlohmann::json::parse(replayCommands, nullptr, false);
        if (j.is_discarded() || !j.is_array()) {
            LogError("Replay commands is not a valid JSON array:\n```\n{}```\n", replayCommands);
            return;
        }

        for (const auto& item : j) {
            if (!item.is_object()) {
                LogError("Replay command is not a valid object:\n```\n{}```\n", item.dump());
                return;
            }

            DoHandleClientMessage(item.dump());
        }

        threadPool.wait();
    }

    auto LanguageServer::DoHandleClientMessage(StringView messagePayload) -> void
    {
        LogClientMessage(messagePayload);

        auto j = nlohmann::json::parse(messagePayload, nullptr, false);
        if (j.is_discarded()) {
            LogError("Client message is not a valid JSON:\n```\n{}```\n", messagePayload);
            return;
        }

        std::string method;
        if (auto itMethod = j.find("method"); itMethod != j.end() && itMethod->is_string()) {
            method = *itMethod;
        }
        else {
            LogError("JSON-RPC method must be a valid string.");
            return;
        }

        if (auto it = dispatcherMap.find(method); it != dispatcherMap.end()) {
            it->second(*this, j);
        }
        else {
            // Ignore unknown methods
            LogWarn("Unknown JSON-RPC method: {}", method);
        }
    }

    auto LanguageServer::DoHandleServerResponse(int requestId, nlohmann::json result, bool isError) -> void
    {
        nlohmann::json rpcBlob;
        rpcBlob["jsonrpc"] = "2.0";
        rpcBlob["id"]      = requestId;
        if (isError) {
            rpcBlob["error"] = std::move(result);
        }
        else {
            rpcBlob["result"] = std::move(result);
        }

        auto payload = rpcBlob.dump();
        transport->PushMessage(StringView{payload});
    }

    auto LanguageServer::DoHandleServerNotification(const char* method, nlohmann::json params) -> void
    {
        nlohmann::json rpcBlob;
        rpcBlob["jsonrpc"] = "2.0";
        rpcBlob["method"]  = method;
        rpcBlob["params"]  = std::move(params);

        auto payload = rpcBlob.dump();
        transport->PushMessage(StringView{payload});
    }

    auto LanguageServer::AddClientMessageHandler(
        StringView methodName, std::function<auto(LanguageServer&, const nlohmann::json&)->void> handler) -> void
    {
        auto [it, inserted] = dispatcherMap.insert_or_assign(methodName.Str(), std::move(handler));
        GLSLD_ASSERT(inserted);
    }
} // namespace glsld