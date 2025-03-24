#include "Server/Config.h"
#include "Support/JsonSerializer.h"

namespace glsld
{
    auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>
    {
        auto json = nlohmann::json::parse(configFile, nullptr, false);
        if (!json.is_null()) {
            LanguageServerConfig result;
            if (JsonSerializer<LanguageServerConfig>::Deserialize(result, json)) {
                return result;
            }
        }

        return std::nullopt;
    }
} // namespace glsld