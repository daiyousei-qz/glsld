#pragma once
#include "Ast/AstVisitor.h"

#include "Server/LanguageQueryInfo.h"

namespace glsld
{
    template <typename Derived, typename ResultType = void>
    class LanguageQueryVisitor : public AstVisitor<Derived, ResultType>
    {
    private:
        const LanguageQueryInfo& info;

    public:
        LanguageQueryVisitor(const LanguageQueryInfo& info) : info(info)
        {
        }

        auto GetInfo() const -> const LanguageQueryInfo&
        {
            return info;
        }

    protected:
        auto TraverseNodeContains(const AstNode& node, TextPosition position) const -> AstVisitPolicy
        {
            // TODO: pre-convert position to a token index for performance
            if (info.ContainsPositionExtended(node, position)) {
                return AstVisitPolicy::Traverse;
            }
            else if (info.PrecedesPosition(node, position)) {
                return AstVisitPolicy::Leave;
            }
            else {
                return AstVisitPolicy::Halt;
            }
        }

        auto TraverseNodeUntil(const AstNode& node, TextPosition position) const -> AstVisitPolicy
        {
            // TODO: pre-convert position to a token index for performance
            if (info.SucceedsPosition(node, position)) {
                return AstVisitPolicy::Halt;
            }
            else {
                return AstVisitPolicy::Traverse;
            }
        }

        auto TraverseNodeOverlaps(const AstNode& node, TextRange range) const -> AstVisitPolicy
        {
            // TODO: optimize by pre-compute token ranges
            if (range.Overlaps(info.LookupExpandedTextRangeExtended(node.GetSyntaxRange()))) {
                return AstVisitPolicy::Traverse;
            }
            else if (info.PrecedesPosition(node, range.end)) {
                return AstVisitPolicy::Leave;
            }
            else {
                return AstVisitPolicy::Halt;
            }
        }
    };
} // namespace glsld