#include "LanguageService.h"
#include "ModuleVisitor.h"

namespace glsld
{
    class DocumentColorVisitor : public ModuleVisitor<DocumentColorVisitor>
    {
    public:
        auto VisitAstBinaryExpr(AstBinaryExpr& expr) -> void
        {
            // Only work for "=", "+=", "-=", "*=", "/="
            switch (expr.GetOperator()) {
            case BinaryOp::Assign:
            case BinaryOp::AddAssign:
            case BinaryOp::SubAssign:
            case BinaryOp::MulAssign:
            case BinaryOp::DivAssign:
                break;
            default:
                return;
            }

            auto lhs = expr.GetLeftOperandExpr()->As<AstNameAccessExpr>();
            if (!lhs) {
                return;
            }

            auto accessName = StringView{lhs->GetAccessName().text.StrView()};
            if (!accessName.Contains("color") || !accessName.Contains("Color") || !accessName.Contains("COLOR")) {
                // FIXME: any other acceptable name for color?
                return;
            }

            if (!lhs->GetDeducedType()->IsSameWith(GlslBuiltinType::Ty_vec3) &&
                !lhs->GetDeducedType()->IsSameWith(GlslBuiltinType::Ty_vec4)) {
                // FIXME: any other acceptable type? Like dvec3?
                return;
            }

            // FIXME: need constant folding
            GLSLD_NO_IMPL();
        }

        auto Export() -> std::vector<lsp::ColorInformation>
        {
            return std::move(result);
        }

    private:
        std::vector<lsp::ColorInformation> result;
    };

    auto ComputeDocumentColor(const CompilerObject& compileResult) -> std::vector<lsp::ColorInformation>
    {
        // return {lsp::ColorInformation{
        //     .range = {.start = {.line = 0, .character = 0}, .end{.line = 0, .character = 4}},
        //     .color = {1, 0, 0, 1},
        // }};
        return {};
    }
} // namespace glsld