#pragma once
#include "Ast/Decl.h"

namespace glsld
{
    class SourceReconstructionBuilder
    {
    private:
        fmt::memory_buffer buffer;

        template <typename... Ts>
        auto Append(fmt::format_string<Ts...> fmt, Ts&&... args) -> void
        {
            fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Ts>(args)...);
        }

        auto AppendInitializer(const AstInitializer& initializer) -> void;

        auto AppendArraySpec(const AstArraySpec& arraySpec) -> void;

        auto AppendTypeQualifierSeq(const AstTypeQualifierSeq& typeQualifierSeq) -> void;

        auto AppendQualType(const AstQualType& qualType) -> void;

        auto AppendDeclarator(const AstSyntaxToken& nameToken, const AstArraySpec* arraySpec,
                              const AstInitializer* initializer) -> void;

        auto AppendQualTypeDeclarator(const AstQualType& qualType, const AstSyntaxToken& nameToken,
                                      const AstArraySpec* arraySpec, const AstInitializer* initializer) -> void;

        auto AppendParamDecl(const AstParamDecl& paramDecl) -> void;

        auto AppendFunctionDecl(const AstFunctionDecl& decl) -> void;

        auto AppendStructDecl(const AstStructDecl& decl) -> void;

        auto AppendInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void;

    public:
        SourceReconstructionBuilder() = default;

        auto Print(const AstDecl& decl) -> std::string;
    };
} // namespace glsld