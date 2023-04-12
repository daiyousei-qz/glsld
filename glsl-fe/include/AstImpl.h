#pragma once
#include "AstBase.h"
#include "AstPayload.h"

#if defined(_MSC_VER)
#define GLSLD_MSVC_EMPTY_BASES __declspec(empty_bases)
#else
#define GLSLD_MSVC_EMPTY_BASES
#endif

namespace glsld
{
    template <typename AstType>
    class GLSLD_MSVC_EMPTY_BASES AstImpl : public AstNodeTrait<AstType>::ParentType, public AstPayload<AstType>
    {
    public:
        auto DumpPayloadData() const -> std::string
        {
            using ParentType = typename AstNodeTrait<AstType>::ParentType;

            if constexpr (requires { ParentType::DumpPayloadData(); }) {
                std::string parentPayloadText = ParentType::DumpPayloadData();
                std::string payloadText       = AstPayload<AstType>::DumpPayloadData();

                if (parentPayloadText.empty()) {
                    return payloadText;
                }
                else if (payloadText.empty()) {
                    return parentPayloadText;
                }
                else {
                    return parentPayloadText + "; " + payloadText;
                }
            }
            else {
                return AstPayload<AstType>::DumpPayloadData();
            }
        }
    };
} // namespace glsld