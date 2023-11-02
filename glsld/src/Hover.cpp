#include "LanguageService.h"
#include "Markdown.h"
#include "SourceText.h"
#include "StandardDocumentation.h"

namespace glsld
{
    struct HoverContent
    {
        SymbolAccessType type;
        std::string name;
        std::string description;
        std::string documentation;
        std::string code;

        // We can't resolve the AST node that declares the symbol.
        bool unknown = false;
    };

    static auto ComputeHoverText(const HoverContent& hover) -> std::string
    {
        MarkdownBuilder builder;

        // Header explaining what the hover symbol is
        builder.Append("### ");
        if (hover.unknown) {
            builder.Append("Unknown ");
        }
        switch (hover.type) {
        case SymbolAccessType::LayoutQualifier:
            builder.Append("Layout Qualifier");
            break;
        case SymbolAccessType::Variable:
            builder.Append("Variable");
            break;
        case SymbolAccessType::Swizzle:
            builder.Append("Swizzle");
            break;
        case SymbolAccessType::MemberVariable:
            builder.Append("Member Variable");
            break;
        case SymbolAccessType::Parameter:
            builder.Append("Parameter");
            break;
        case SymbolAccessType::Function:
            builder.Append("Function");
            break;
        case SymbolAccessType::Type:
            builder.Append("Type");
            break;
        case SymbolAccessType::InterfaceBlock:
            builder.Append("Interface Block");
            break;
        case SymbolAccessType::InterfaceBlockInstance:
            builder.Append("Interface Block Instance");
            break;
        case SymbolAccessType::Unknown:
            GLSLD_UNREACHABLE();
        }
        builder.Append(fmt::format(" `{}`", hover.name));

        // Description
        if (!hover.description.empty()) {
            builder.AppendRuler();
            builder.AppendParagraph(hover.description);
        }

        // Documentation
        if (!hover.documentation.empty()) {
            builder.AppendRuler();
            builder.AppendParagraph(hover.documentation);
        }

        // Reconstructed Code
        if (!hover.code.empty()) {
            builder.AppendRuler();
            builder.AppendCodeBlock(hover.code, "glsl");
        }

        return builder.Export();
    }

    static auto CreateHoverContent(const SymbolAccessInfo& accessInfo) -> std::optional<HoverContent>
    {
        if (!accessInfo.token.IsIdentifier()) {
            return std::nullopt;
        }

        StringView name = accessInfo.token.text.StrView();
        if (!accessInfo.symbolDecl.IsValid()) {
            bool isUnknown = accessInfo.symbolType != SymbolAccessType::Swizzle &&
                             accessInfo.symbolType != SymbolAccessType::LayoutQualifier;

            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = "",
                .code        = "",
                .unknown     = isUnknown,
            };
        }

        auto decl = accessInfo.symbolDecl.GetDecl();
        std::string descriptionBuffer;
        std::string codeBuffer;
        if (auto funcDecl = decl->As<AstFunctionDecl>();
            funcDecl && accessInfo.symbolType == SymbolAccessType::Function) {
            ReconstructSourceText(codeBuffer, *funcDecl);
            return HoverContent{
                .type          = SymbolAccessType::Function,
                .name          = name.Str(),
                .description   = std::move(descriptionBuffer),
                .documentation = QueryFunctionDocumentation(name).Str(),
                .code          = std::move(codeBuffer),
            };
        }
        else if (auto paramDecl = decl->As<AstParamDecl>();
                 paramDecl && accessInfo.symbolType == SymbolAccessType::Parameter) {
            ReconstructSourceText(codeBuffer, *paramDecl);
            return HoverContent{
                .type        = SymbolAccessType::Parameter,
                .name        = name.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto varDecl = decl->As<AstVariableDecl>();
                 varDecl && accessInfo.symbolType == SymbolAccessType::Variable) {
            ReconstructSourceText(codeBuffer, *varDecl, accessInfo.symbolDecl.GetIndex());
            return HoverContent{
                .type        = SymbolAccessType::Variable,
                .name        = name.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto memberDecl = decl->As<AstFieldDecl>();
                 memberDecl && accessInfo.symbolType == SymbolAccessType::MemberVariable) {
            ReconstructSourceText(codeBuffer, *memberDecl, accessInfo.symbolDecl.GetIndex());
            return HoverContent{
                .type        = SymbolAccessType::MemberVariable,
                .name        = name.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto structDecl = decl->As<AstStructDecl>();
                 structDecl && accessInfo.symbolType == SymbolAccessType::Type) {
            ReconstructSourceText(codeBuffer, *structDecl);
            return HoverContent{
                .type        = SymbolAccessType::Type,
                .name        = name.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }
        else if (auto blockDecl = decl->As<AstInterfaceBlockDecl>();
                 blockDecl && (accessInfo.symbolType == SymbolAccessType::InterfaceBlock ||
                               accessInfo.symbolType == SymbolAccessType::InterfaceBlockInstance)) {
            ReconstructSourceText(codeBuffer, *blockDecl);
            return HoverContent{
                .type        = accessInfo.symbolType,
                .name        = name.Str(),
                .description = std::move(descriptionBuffer),
                .code        = std::move(codeBuffer),
            };
        }

        return std::nullopt;
    }

    auto ComputeHover(const LanguageQueryProvider& provider, lsp::Position position) -> std::optional<lsp::Hover>
    {
        const auto& compilerObject = provider.GetCompilerObject();

        auto accessInfo = provider.LookupSymbolAccess(FromLspPosition(position));
        if (accessInfo) {

            // Decl token that's either builtin or unknown
            if (accessInfo->symbolType == SymbolAccessType::Unknown) {
                return std::nullopt;
            }

            // FIXME: is the following needed?
            if (accessInfo->token.IsKeyword()) {
                // Don't provide hover for keyword
                return std::nullopt;
            }

            if (auto hoverContent = CreateHoverContent(*accessInfo)) {
                auto hoverRange = provider.GetExpandedTextRange(accessInfo->token);
                return lsp::Hover{
                    .contents = ComputeHoverText(*hoverContent),
                    .range    = ToLspRange(hoverRange),
                };
            }
        }

        return std::nullopt;
    }

} // namespace glsld