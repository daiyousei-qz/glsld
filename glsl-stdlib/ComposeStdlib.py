from StdlibBuilder import StdlibBuilder
from BuiltinFunction import *
from BuiltinVariables import *

import os

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))

def escapeString(string: str):
    return string.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")

def addAllBuiltinVariables(builder: StdlibBuilder):
    addBuiltinConstant(builder)
    addBuiltinVariablesForVertexShader(builder)
    addBuiltinVariablesForFragmentShader(builder)
    addBuiltinVariablesForComputeShader(builder)
    addBuiltinVariablesForGeometryShader(builder)
    addBuiltinVariablesForTessControlShader(builder)
    addBuiltinVariablesForTessEvaluationShader(builder)
    addBuiltinVariablesForRayTracingEXTShader(builder)
    addBuiltinVariablesForRayTracingNVShader(builder)

def addAllBuiltinFunctions(builder: StdlibBuilder):
    addTrigonometryFunctions(builder)
    addExponentialFunctions(builder)
    addCommonFunctions(builder)
    addFPPackUnpackFunctions(builder)
    addGeometricFunctions(builder)
    addMatrixFunctions(builder)
    addVectorRelationalFunctions(builder)
    addIntegerFunctions(builder)
    addTextureFunctions(builder)
    addAtomicCounterFunctions(builder)
    addAtomicMemoryFunctions(builder)
    addImageFunctions(builder)
    addGeometryShaderFunctions(builder)
    addFragmentProcessingFunctions(builder)
    addNoiseFunctions(builder)
    addShaderInvocationFunctions(builder)
    addShaderMemoryFunctions(builder)
    addSubpassInputFunctions(builder)
    addShaderInvocationGroupFunctions(builder)
    
def addAllBuiltinForExtensions(builder: StdlibBuilder):
    from Extension import addBuiltinFor_GL_EXT_ray_query
    addBuiltinFor_GL_EXT_ray_query(builder)

    from Extension import addBuiltinFor_GL_KHR_memory_scope_semantics
    addBuiltinFor_GL_KHR_memory_scope_semantics(builder)

    from Extension import addBuiltinFor_GL_NV_shader_sm_builtins
    addBuiltinFor_GL_NV_shader_sm_builtins(builder)

def main():
    builder = StdlibBuilder()
    addAllBuiltinVariables(builder)
    addAllBuiltinFunctions(builder)
    addAllBuiltinForExtensions(builder)

    with open(
        os.path.join(SCRIPT_PATH, "../glsld-core/include/Language/Stdlib.Generated.h"),
        "w",
    ) as file:
        file.write("// clang-format off\n")
        file.write("#include <string_view>\n")
        file.write("\n")

        file.write("namespace glsld {\n")
        file.write("inline std::string_view GlslStdlibText = R\"glsl(\n")
        file.write(builder.build())
        file.write(")glsl\";\n")
        file.write("} // namespace glsld\n")
        file.write("// clang-format on\n")

if __name__ == "__main__":
    main()