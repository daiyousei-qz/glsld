#include "Basic/StringView.h"

#include "Server/StandardDocumentation.h"

#include <unordered_map>

namespace glsld
{
    auto QueryLocationQualifierDocumentation(StringView name) -> StringView
    {
        static const std::unordered_map<StringView, StringView> documentations = {
            // Tesselation
            //

            {
                "triangles",
                "The `primitive-mode` is used to specify a tessellation primitive mode to be used by the tessellation "
                "primitive generator.",
            },
            {
                "quads",
                "The `primitive-mode` is used to specify a tessellation primitive mode to be used by the tessellation "
                "primitive generator.",
            },
            {
                "isolines",
                "The `primitive-mode` is used to specify a tessellation primitive mode to be used by the tessellation "
                "primitive generator.",
            },
            {
                "equal_spacing",
                "`equal_spacing` specifies that edges should be divided into a collection of equal-sized segments",
            },
            {
                "fractional_even_spacing",
                "`fractional_even_spacing` specifies that edges should be divided into an even number of equal-length "
                "segments plus two additional shorter \"fractional\" segments",
            },
            {
                "fractional_odd_spacing",
                "`fractional_odd_spacing` specifies that edges should be divided into an odd number of equal-length "
                "segments plus two additional shorter \"fractional\" segments",
            },
            {
                "cw",
                "`cw` indicate clockwise triangles,  If the tessellation primitive generator does not produce "
                "triangles, the order is ignored",
            },
            {
                "ccw",
                "`ccw` indicate counter-clockwise triangles,  If the tessellation primitive generator does not produce "
                "triangles, the order is ignored",
            },
            {
                "point_mode",
                "`point_mode` indicates that the tessellation primitive generator should produce one point for each "
                "distinct vertex in the subdivided primitive, rather than generating lines or triangles",
            },
            {
                "points",
                "`points` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "lines",
                "`lines` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "lines_adjacency",
                "`lines_adjacency` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "triangles",
                "`triangles` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "triangles_adjacency",
                "`triangles_adjacency` is used to specify the type of input primitive accepted by the geometry shader",
            },
            {
                "invocations",
                "`invocations = layout-qualifier-value` is used to specify the number of times the geometry shader "
                "executable is invoked for each input primitive received. Invocation count declarations are optional. "
                "If no invocation count is declared in any geometry shader in a program, the geometry shader will be "
                "run once for each input primitive.",
            },

            // Fragment
            //

            // {
            //     "origin_upper_left",
            //     "`origin_upper_left` ",
            // },
            // {
            //     "pixel_center_integer",
            //     "`pixel_center_integer` ",
            // },
            // {
            //     "early_fragment_tests",
            //     "`early_fragment_tests` ",
            // },

            // Compute
            //
            {
                "local_size_x",
                "`local_size_x` is used to declare a fixed workgroup size by the compute shader in the first "
                "dimension. If a shader does not specify a size for one of the dimensions, that dimension will have a "
                "size of 1.",
            },
            {
                "local_size_y",
                "`local_size_y` is used to declare a fixed workgroup size by the compute shader in the second "
                "dimension. If a shader does not specify a size for one of the dimensions, that dimension will have a "
                "size of 1.",
            },
            {
                "local_size_y",
                "`local_size_y` is used to declare a fixed workgroup size by the compute shader in the third "
                "dimension. If a shader does not specify a size for one of the dimensions, that dimension will have a "
                "size of 1.",
            },

            // Output
            //
            {
                "location",
                "`location` ",
            },
            {
                "component",
                "`component` ",
            },
            {
                "index",
                "`index` ",
            },

            // Transform Feedback
            //
            {
                "xfb_buffer",
                "The `xfb_buffer` qualifier specifies which transform feedback buffer will capture outputs selected "
                "with `xfb_offset`. The `xfb_buffer` qualifier can be applied to the qualifier out, to output "
                "variables, to output blocks, and to output block members.",
            },
            {
                "xfb_offset",
                "`xfb_offset` The xfb_offset qualifier assigns a byte offset within a transform feedback buffer. Only "
                "variables, block members, or blocks can be qualified with xfb_offset.",
            },
            {
                "xfb_stride",
                "`xfb_stride` The xfb_stride qualifier specifies how many bytes are consumed by each captured vertex. "
                "It applies to the transform feedback buffer for that declaration, whether it is inherited or "
                "explicitly declared.",
            },

            // Tessellation Control Outputs
            //
            {
                "vertices",
                "`vertices` The identifier vertices specifies the number of vertices in the output patch produced by "
                "the tessellation control shader, which also specifies the number of times the tessellation control "
                "shader is invoked. It is a compile- or link-time error for the output vertex count to be less than or "
                "equal to zero, or greater than the implementation-dependent maximum patch size.",
            },

            // Geometry Outputs
            //
            // {
            //     "points",
            //     "`points` ",
            // },
            // {
            //     "line_strip",
            //     "`line_strip` ",
            // },
            // {
            //     "triangle_strip",
            //     "`triangle_strip` ",
            // },
            // {
            //     "max_vertices",
            //     "`max_vertices` ",
            // },
            // {
            //     "stream",
            //     "`stream` ",
            // },

            // Fragment Outputs
            //
            // {
            //     "depth_any",
            //     "`depth_any` ",
            // },
            // {
            //     "depth_greater",
            //     "`depth_greater` ",
            // },
            // {
            //     "depth_less",
            //     "`depth_less` ",
            // },
            // {
            //     "depth_unchanged",
            //     "`depth_unchanged` ",
            // },
        };

        if (auto it = documentations.find(name); it != documentations.end()) {
            return it->second;
        }
        else {
            return "";
        }
    }
} // namespace glsld