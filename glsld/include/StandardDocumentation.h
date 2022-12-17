#pragma once
#include "Common.h"

#include <string>

namespace glsld
{
    auto QueryLocationQualifierDocumentation(StringView name) -> StringView;

    // FIXME: do not hard code this, but rather make it generic and work for user code
    auto QueryFunctionDocumentation(StringView name) -> StringView;
} // namespace glsld