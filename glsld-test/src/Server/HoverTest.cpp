#include "ServerTestFixture.h"

#include "SourceText.h"
#include "Hover.h"

using namespace glsld;

TEST_CASE_METHOD(ServerTestFixture, "HoverRange")
{
    auto sourceText = R"(
        void ^pos.begin^ma^pos.mid^in^pos.end^()
        {
        }
    )";

    auto ctx = CompileLabelledSource(sourceText);

    auto hover1 = ComputeHover(ctx.GetProvider(), ToLspPosition(ctx.GetPosition("pos.begin")));
    REQUIRE(hover1.has_value());
    REQUIRE(FromLspRange(hover1->range) == ctx.GetRange("pos.begin", "pos.end"));

    auto hover2 = ComputeHover(ctx.GetProvider(), ToLspPosition(ctx.GetPosition("pos.mid")));
    REQUIRE(hover2.has_value());
    REQUIRE(FromLspRange(hover1->range) == ctx.GetRange("pos.begin", "pos.end"));

    auto hover3 = ComputeHover(ctx.GetProvider(), ToLspPosition(ctx.GetPosition("pos.end")));
    REQUIRE(!hover3.has_value());
}