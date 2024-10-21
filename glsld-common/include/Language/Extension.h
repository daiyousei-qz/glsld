#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

#include <vector>
#include <optional>

namespace glsld
{
    enum class ExtensionBehavior
    {
        Disable,
        Enable,
        Warn,
        Require,
    };

    enum class ExtensionId
    {
#define DECL_EXTENSION(EXTENSION_NAME) EXTENSION_NAME,
#include "GlslExtension.inc"
#undef DECL_EXTENSION
    };

    inline auto GetExtensionName(ExtensionId id) -> StringView
    {
        switch (id) {
#define DECL_EXTENSION(EXTENSION_NAME)                                                                                 \
    case ExtensionId::EXTENSION_NAME:                                                                                  \
        return #EXTENSION_NAME;
#include "GlslExtension.inc"

#undef DECL_EXTENSION
        }

        GLSLD_UNREACHABLE();
    }

    inline auto ParseExtensionName(StringView name) -> std::optional<ExtensionId>
    {
#define DECL_EXTENSION(EXTENSION_NAME)                                                                                 \
    if (name == #EXTENSION_NAME) {                                                                                     \
        return ExtensionId::EXTENSION_NAME;                                                                            \
    }
#include "GlslExtension.inc"
#undef DECL_EXTENSION

        return std::nullopt;
    }

    class ExtensionStatus
    {
    private:
#define DECL_EXTENSION(EXTENSION_NAME) bool EXTENSION_NAME##_enable : 1 = false;
#include "GlslExtension.inc"
#undef DECL_EXTENSION

    public:
        ExtensionStatus() = default;

        auto Clear() -> void
        {
#define DECL_EXTENSION(EXTENSION_NAME) EXTENSION_NAME##_enable = false;
#include "GlslExtension.inc"
#undef DECL_EXTENSION
        }

        auto EnableExtension(ExtensionId id) -> void
        {
            switch (id) {
#define DECL_EXTENSION(EXTENSION_NAME)                                                                                 \
    case ExtensionId::EXTENSION_NAME:                                                                                  \
        EXTENSION_NAME##_enable = true;                                                                                \
        return;
#include "GlslExtension.inc"
#undef DECL_EXTENSION
            }

            GLSLD_UNREACHABLE();
        }

        auto IsExtensionEnabled(ExtensionId id) const -> bool
        {
            switch (id) {
#define DECL_EXTENSION(EXTENSION_NAME)                                                                                 \
    case ExtensionId::EXTENSION_NAME:                                                                                  \
        return EXTENSION_NAME##_enable;
#include "GlslExtension.inc"
#undef DECL_EXTENSION
            }

            GLSLD_UNREACHABLE();
        }
    };
} // namespace glsld