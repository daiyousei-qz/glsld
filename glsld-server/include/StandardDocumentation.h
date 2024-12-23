#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

namespace glsld
{
    auto QueryLocationQualifierDocumentation(StringView name) -> StringView;

    // FIXME: do not hard code this, but rather make it generic and work for user code
    auto QueryFunctionDocumentation(StringView name) -> StringView;
} // namespace glsld