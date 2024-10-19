import os

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))


def EscapeString(string: str):
    return string.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")


def LoadFile(filename: str, output: list):
    with open(os.path.join(SCRIPT_PATH, filename), "r") as file:
        for line in file.readlines():
            output.append('"' + EscapeString(line) + '"')


result = []
LoadFile("builtin-functions/AtomicCounterFunctions.glsl", result)
LoadFile("builtin-functions/BarrierFunctions.glsl", result)
LoadFile("builtin-functions/CommonFunctions.glsl", result)
LoadFile("builtin-functions/CommonFunctions2.glsl", result)
LoadFile("builtin-functions/DerivativeFunctions.glsl", result)
LoadFile("builtin-functions/ExponentialFunctions.glsl", result)
LoadFile("builtin-functions/GeometricFunctions.glsl", result)
LoadFile("builtin-functions/ImageFunctions.glsl", result)
LoadFile("builtin-functions/IntegerFunctions.glsl", result)
LoadFile("builtin-functions/MatrixFunctions.glsl", result)
LoadFile("builtin-functions/MemoryBarrierFunctions.glsl", result)
LoadFile("builtin-functions/NoiseFunctions.glsl", result)
LoadFile("builtin-functions/TextureGatherFunctions.glsl", result)
LoadFile("builtin-functions/TextureLookupFunctions.glsl", result)
LoadFile("builtin-functions/TextureQueryFunctions.glsl", result)
LoadFile("builtin-functions/TrigonometryFunctions.glsl", result)
LoadFile("builtin-functions/VectorRationalFunctions.glsl", result)

LoadFile("stage/ComputeShader.glsl", result)
LoadFile("stage/FragmentShader.glsl", result)
LoadFile("stage/VertexShader.glsl", result)
LoadFile("stage/RayTracingShaderEXT.glsl", result)
LoadFile("stage/RayTracingShaderNV.glsl", result)

LoadFile("extension/GL_EXT_ray_query.glsl", result)


with open(os.path.join(SCRIPT_PATH, "Stdlib.Generated.h"), "w") as file:
    file.write("// clang-format off\n")
    for line in result:
        file.write(line + "\n")
    file.write("// clang-format on\n")
