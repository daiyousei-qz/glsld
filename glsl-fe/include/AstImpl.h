#pragma once
#include "AstBase.h"
#include "AstPayload.h"

#if defined(_MSC_VER)
#define MSVC_EMPTY_BASES __declspec(empty_bases)
#else
#define MSVC_EMPTY_BASES
#endif

namespace glsld
{
    template <typename AstType>
    class MSVC_EMPTY_BASES AstImpl : public AstNodeTrait<AstType>::ParentType, public AstPayload<AstType>
    {
    };
} // namespace glsld