#include "Server/LanguageServer.h"
#include "Server/LanguageService.h"
#include "Server/Protocol.h"
#include "Support/StringView.h"

#include <cctype>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace glsld
{
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

    LanguageServer::LanguageServer(const LanguageServerConfig& config) : config(config)
    {
        logger = CreateLogger(config.loggingLevel);

        language  = std::make_unique<LanguageService>(*this);
        transport = CreateStdioTextTransport();

        InitializeClientMessageHandlers();
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
        while (running && PullMessage()) {
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

    auto LanguageServer::PullMessage() -> bool
    {
        // We have a single thread loop here, no need to lock
        // std::lock_guard<std::mutex> lock{transportMutex};

        size_t payloadLength = 0;
        while (true) {
            StringView headerView;
            if (auto headerLine = transport->ReadLine(); headerLine.has_value()) {
                headerView = headerLine->Trim();
            }
            else {
                LogError("Failed to read LSP message header.");
                return false;
            }

            LogDebug("Received LSP message header line: `{}`", headerView);
            if (headerView.Empty()) {
                // Empty line indicates end of headers, aka. start of payload.
                break;
            }

            if (headerView.StartWith("Content-Length: ")) {
                auto lengthView = headerView.Drop(16);
                if (std::from_chars(lengthView.data(), lengthView.data() + lengthView.Size(), payloadLength).ec !=
                    std::errc()) {
                    LogError("Failed to parse Content-Length header.");
                    return false;
                }
            }
            else if (headerView.StartWith("Content-Type: ")) {
                // do nothing...
            }
            else {
                LogWarn("Unknown LSP message header: {}", headerView);
                // We ignore any unknown header fields
            }
        }

        if (payloadLength == 0) {
            LogError("LSP message payload length is missing or incorrectly set to zero.");
            return false;
        }
        auto payload = transport->Read(payloadLength);
        if (!payload.has_value()) {
            LogError("Failed to read LSP message payload.");
            return false;
        }

        LogDebug("Received LSP message payload:\n```\n{}\n```", *payload);
        DoHandleClientMessage(*payload);
        return true;
    }

    auto LanguageServer::PushMessage(StringView payload) -> bool
    {
        std::lock_guard<std::mutex> lock{serverOutputMutex};

        // TODO: optimize this
        std::string header = fmt::format("Content-Length: {}\r\n\r\n", payload.Size());
        LogDebug("Sending LSP message header:\n```\n{}```", header);
        LogDebug("Sending LSP message payload:\n```\n{}\n```", payload);

        if (!transport->Write(header)) {
            LogError("Failed to write LSP message header.");
            return false;
        }
        if (!transport->Write(payload)) {
            LogError("Failed to write LSP message payload.");
            return false;
        }
        if (!transport->Flush()) {
            LogError("Failed to flush LSP message.");
            return false;
        }

        return true;
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

        if (auto it = handlerDispatchMap.Find(method); it != handlerDispatchMap.end()) {
            std::invoke(it->second, *this, j);
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

        if (!PushMessage(payload)) {
            LogError("Failed to push server response:\n{}", payload);
            Shutdown();
        }
    }

    auto LanguageServer::DoHandleServerNotification(const char* method, nlohmann::json params) -> void
    {
        nlohmann::json rpcBlob;
        rpcBlob["jsonrpc"] = "2.0";
        rpcBlob["method"]  = method;
        rpcBlob["params"]  = std::move(params);

        auto payload = rpcBlob.dump();

        if (!PushMessage(payload)) {
            LogError("Failed to push server notification:\n{}", payload);
            Shutdown();
        }
    }

    template <typename ParamType>
    using RequestHandlerType = void (LanguageService::*)(int requestId, ParamType params);

    template <typename ParamType>
    using NotificationHandlerType = void (LanguageService::*)(ParamType params);

    auto LanguageServer::InitializeClientMessageHandlers() -> void
    {
        const auto createRequestHandler = []<typename ParamType>(RequestHandlerType<ParamType> handler) {
            return [handler](LanguageServer& server, const nlohmann::json& rpcBlob) {
                int requestId;
                if (auto itReqId = rpcBlob.find("id"); itReqId != rpcBlob.end() && itReqId->is_number_integer()) {
                    requestId = *itReqId;
                }
                else {
                    server.LogError("JSON-RPC request ID must be a valid integer.");
                    return;
                }

                ParamType params = {};
                if (auto itParam = rpcBlob.find("params");
                    JsonSerializer<ParamType>::Deserialize(params, itParam != rpcBlob.end() ? *itParam : nullptr)) {
                    std::invoke(handler, server.language.get(), requestId, std::move(params));
                }
                else {
                    server.LogError("Failed to deserialize JSON-RPC request parameters.");
                }
            };
        };

        const auto createNotificationHandler = []<typename ParamType>(NotificationHandlerType<ParamType> handler) {
            return [handler](LanguageServer& server, const nlohmann::json& rpcBlob) {
                ParamType params = {};
                if (auto itParam = rpcBlob.find("params");
                    JsonSerializer<ParamType>::Deserialize(params, itParam != rpcBlob.end() ? *itParam : nullptr)) {
                    std::invoke(handler, server.language.get(), std::move(params));
                }
                else {
                    server.LogError("Failed to deserialize JSON-RPC notification parameters.");
                }
            };
        };

        handlerDispatchMap[lsp::LSPMethod_Initialize]  = createRequestHandler(&LanguageService::Initialize);
        handlerDispatchMap[lsp::LSPMethod_Initialized] = createNotificationHandler(&LanguageService::Initialized);
        handlerDispatchMap[lsp::LSPMethod_SetTrace]    = createNotificationHandler(&LanguageService::SetTrace);
        handlerDispatchMap[lsp::LSPMethod_Shutdown]    = createRequestHandler(&LanguageService::Shutdown);
        handlerDispatchMap[lsp::LSPMethod_Exit]        = createNotificationHandler(&LanguageService::Exit);

        handlerDispatchMap[lsp::LSPMethod_DocumentSymbol] = createRequestHandler(&LanguageService::DocumentSymbol);
        handlerDispatchMap[lsp::LSPMethod_SemanticTokensFull] =
            createRequestHandler(&LanguageService::SemanticTokensFull);
        handlerDispatchMap[lsp::LSPMethod_SemanticTokensFullDelta] =
            createRequestHandler(&LanguageService::SemanticTokensFullDelta);
        handlerDispatchMap[lsp::LSPMethod_Completion]    = createRequestHandler(&LanguageService::Completion);
        handlerDispatchMap[lsp::LSPMethod_SignatureHelp] = createRequestHandler(&LanguageService::SignatureHelp);
        handlerDispatchMap[lsp::LSPMethod_Hover]         = createRequestHandler(&LanguageService::Hover);
        handlerDispatchMap[lsp::LSPMethod_References]    = createRequestHandler(&LanguageService::References);
        handlerDispatchMap[lsp::LSPMethod_Definition]    = createRequestHandler(&LanguageService::Definition);
        handlerDispatchMap[lsp::LSPMethod_InlayHint]     = createRequestHandler(&LanguageService::InlayHint);

        handlerDispatchMap[lsp::LSPMethod_DidOpenTextDocument] =
            createNotificationHandler(&LanguageService::DidOpenTextDocument);
        handlerDispatchMap[lsp::LSPMethod_DidChangeTextDocument] =
            createNotificationHandler(&LanguageService::DidChangeTextDocument);
        handlerDispatchMap[lsp::LSPMethod_DidCloseTextDocument] =
            createNotificationHandler(&LanguageService::DidCloseTextDocument);
    }

} // namespace glsld