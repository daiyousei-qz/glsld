#include "LanguageServer.h"

#include <string_view>
#include <vector>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

int main()
{
    using namespace std::literals;
    std::this_thread::sleep_for(10s);
    glsld::LanguageServer{}.Run();

    return 0;
}
