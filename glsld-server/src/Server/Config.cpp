#include "Server/Config.h"
#include "Support/JsonSerializer.h"

namespace glsld
{
    auto ParseLanguageServerConfig(StringView configFile) -> std::optional<LanguageServerConfig>
    {
        auto json = nlohmann::json::parse(configFile, nullptr, false, true);
        if (!json.is_discarded()) {
            LanguageServerConfig result;
            if (JsonSerializer<LanguageServerConfig>::Deserialize(result, json)) {
                return result;
            }
        }

        return std::nullopt;
    }
} // namespace glsld